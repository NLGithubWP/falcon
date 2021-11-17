//
// Created by root on 11/11/21.
//

#include <falcon/algorithm/model_builder_helper.h>
#include <falcon/algorithm/vertical/linear_model/linear_regression_builder.h>
#include <falcon/utils/pb_converter/common_converter.h>
#include <falcon/utils/pb_converter/lr_converter.h>
#include <falcon/utils/pb_converter/alg_params_converter.h>
#include <falcon/operator/mpc/spdz_connector.h>
#include <falcon/utils/metric/classification.h>
#include <falcon/utils/math/math_ops.h>
#include <falcon/common.h>
#include <falcon/model/model_io.h>
#include <falcon/utils/logger/logger.h>
#include <falcon/utils/logger/log_alg_params.h>
#include <falcon/utils/metric/regression.h>

#include <ctime>
#include <random>
#include <thread>
#include <future>
#include <iomanip>      // std::setprecision
#include <utility>

#include <glog/logging.h>
#include <Networking/ssl_sockets.h>

LinearRegressionBuilder::LinearRegressionBuilder() = default;

LinearRegressionBuilder::LinearRegressionBuilder(LinearRegressionParams linear_reg_params,
                                                 int m_weight_size,
                                                 std::vector<std::vector<double>> m_training_data,
                                                 std::vector<std::vector<double>> m_testing_data,
                                                 std::vector<double> m_training_labels,
                                                 std::vector<double> m_testing_labels,
                                                 double m_training_accuracy,
                                                 double m_testing_accuracy)
                                                 : ModelBuilder(std::move(m_training_data),
                                                                std::move(m_testing_data),
                                                                std::move(m_training_labels),
                                                                std::move(m_testing_labels),
                                                                m_training_accuracy,
                                                                m_testing_accuracy) {
  batch_size = linear_reg_params.batch_size;
  max_iteration = linear_reg_params.max_iteration;
  converge_threshold = linear_reg_params.converge_threshold;
  with_regularization = linear_reg_params.with_regularization;
  alpha = linear_reg_params.alpha;
  learning_rate = linear_reg_params.learning_rate;
  decay = linear_reg_params.decay;
  penalty = linear_reg_params.penalty;
  optimizer = linear_reg_params.optimizer;
  metric = linear_reg_params.metric;
  dp_budget = linear_reg_params.dp_budget;
  fit_bias = linear_reg_params.fit_bias;
  linear_reg_model = LinearRegressionModel(m_weight_size);
}

LinearRegressionBuilder::~LinearRegressionBuilder() = default;

void LinearRegressionBuilder::backward_computation(const Party &party,
                                                   const std::vector<std::vector<double>> &batch_samples,
                                                   EncodedNumber *predicted_labels,
                                                   const std::vector<int> &batch_indexes,
                                                   int precision,
                                                   EncodedNumber *encrypted_gradients) {
  // retrieve phe pub key and phe random
  djcs_t_public_key* phe_pub_key = djcs_t_init_public_key();
  party.getter_phe_pub_key(phe_pub_key);

  // convert batch loss shares back to encrypted losses
  int cur_batch_size = (int) batch_indexes.size();
  auto* encrypted_batch_losses = new EncodedNumber[cur_batch_size];

  // compute the residual [f_t - y_t] of the batch samples
  // for linear regression, here precision should be
  // (local_weights.precision + sample.precision) = 4 * PHE_FIXED_POINT_PRECISION
  compute_encrypted_residual(party, batch_indexes, training_labels,
                             precision, predicted_labels, encrypted_batch_losses);

  // notice that the update formulas are different for different settings
  // (1) without regularization:
  //    [w_j] = [w_j] - lr * { 2/|B| * \sum_{i=1}^{|B|} [loss_i] * x_{ij} },
  //        where [w_j] precision is: 3 * PHE_FIXED_POINT_PRECISION
  // (2) with l2 regularization:
  //    [w_j] = [w_j] - lr * { 2/|B| * \sum_{i=1}^{|B|} [loss_i] * x_{ij} + 2 * \alpha * [w_j] },
  //        where [w_j] precision is: 3 * PHE_FIXED_POINT_PRECISION
  // (3) with l1 regularization:
  //    [w_j] = [w_j] - lr * { 2/|B| * \sum_{i=1}^{|B|} [loss_i] * x_{ij} + \alpha} (if w_j > 0)
  //    [w_j] = [w_j] - lr * { 2/|B| * \sum_{i=1}^{|B|} [loss_i] * x_{ij} - \alpha} (if w_j < 0)
  //        where [w_j] precision is: 3 * PHE_FIXED_POINT_PRECISION, and
  //        the conditional check need use spdz computation


  // after calculate loss, compute [loss_i]*x_{ij}
  auto** encoded_batch_samples = new EncodedNumber*[cur_batch_size];
  for (int i = 0; i < cur_batch_size; i++) {
    encoded_batch_samples[i] = new EncodedNumber[linear_reg_model.weight_size];
  }

  // the common item is { - lr * { 2/|B| * \sum_{i=1}^{|B|} [loss_i] * x_{ij} }
  double lr_batch = (2 * learning_rate) / this->batch_size;
  for (int i = 0; i < cur_batch_size; i++) {
    for (int j = 0; j < linear_reg_model.weight_size; j++) {
      encoded_batch_samples[i][j].set_double(phe_pub_key->n[0],
                                             0 - lr_batch * batch_samples[i][j],
                                             precision);
    }
  }
  // compute common_gradients, its precision should be
  // 2 * precision = 8 * PHE_FIXED_POINT_PRECISION
  auto* common_gradients = new EncodedNumber[linear_reg_model.weight_size];
  for (int j = 0; j < linear_reg_model.weight_size; j++) {
    EncodedNumber gradient;
    // organize feature j's values of the batch
    auto* batch_feature_j = new EncodedNumber[cur_batch_size];
    for (int i = 0; i < cur_batch_size; i++) {
      batch_feature_j[i] = encoded_batch_samples[i][j];
    }
    // compute homomorphic inner product between feature j and weights
    djcs_t_aux_inner_product(phe_pub_key, party.phe_random, gradient,
                             encrypted_batch_losses, batch_feature_j,
                             cur_batch_size);
    common_gradients[j] = gradient;
    delete [] batch_feature_j;
  }

  // if no regularization
  if (!with_regularization) {
    // if no regularization, then the above common_gradients is what we desire
    for (int j = 0; j < linear_reg_model.weight_size; j++) {
      encrypted_gradients[j] = common_gradients[j];
    }
  } else {
    // inspired by https://towardsdatascience.com/intuitions-on-l1-and-l2-regularisation-235f2db4c261
    auto* regularized_gradients = new EncodedNumber[linear_reg_model.weight_size];
    // if l2 regularization, the other item is { - lr * 2 * \alpha * [w_j] }
    if (penalty == "l2") {
      int common_gradients_precision = abs(common_gradients[0].getter_exponent());
      int weights_precision = abs(linear_reg_model.local_weights[0].getter_exponent());
      int plaintext_precision = common_gradients_precision - weights_precision;
      double constant = 0 - learning_rate * 2 * alpha;
      EncodedNumber encoded_constant;
      encoded_constant.set_double(phe_pub_key->n[0],
                                  constant,
                                  plaintext_precision);
      // first compute the last item of the l2 regularization
      // then, add the second item to the common_gradients
      for (int j = 0; j < linear_reg_model.weight_size; j++) {
        djcs_t_aux_ep_mul(phe_pub_key,
                          regularized_gradients[j],
                          linear_reg_model.local_weights[j],
                          encoded_constant);
        djcs_t_aux_ee_add(phe_pub_key,
                          encrypted_gradients[j],
                          common_gradients[j],
                          regularized_gradients[j]);
      }
    }

    // if l1 regularization, the other item has two forms:
    //    when w_j > 0, it should be { - lr * \alpha }
    //    when w_j < 0, it should be { lr * \alpha }
    // we use spdz to check the sign of [local_weights]
    if (penalty == "l1") {
      // TODO: add spdz computation
      compute_l1_regularized_grad(party, regularized_gradients);
    }
    delete [] regularized_gradients;
  }

  // truncate the gradient precision to make sure it is consistent with [w_j]
  int dest_precision = abs(linear_reg_model.local_weights[0].getter_exponent());
  party.truncate_ciphers_precision(encrypted_gradients,
                                   cur_batch_size,
                                   ACTIVE_PARTY_ID,
                                   dest_precision);

  djcs_t_free_public_key(phe_pub_key);
  delete [] encrypted_batch_losses;
  for (int i = 0; i < cur_batch_size; i++) {
    delete [] encoded_batch_samples[i];
  }
  delete [] encoded_batch_samples;
  delete [] common_gradients;
}

void LinearRegressionBuilder::compute_l1_regularized_grad(const Party &party, EncodedNumber *regularized_gradients) {
  // To compute the sign of each weight in the model, we do the following steps:
  // 1. active party aggregates the weight size vector of all parties and broadcast
  // 2. active party organizes a global encrypted weight vector and broadcast
  // 3. all parties convert the global weight vector into secret shares
  // 4. all parties connect to spdz parties to compute the sign and receive shares
  // 5. all parties convert the secret shares into encrypted sign vector
  // 6. each party picks the corresponding local gradients and multiply the hyper-parameter

  log_info("[compute_l1_regularized_grad]: begin to compute l1 regularized gradients");

  // step 1
  std::vector<int> party_weight_sizes;
  if (party.party_type == falcon::ACTIVE_PARTY) {
    // first set its own weight size and receive other parties' weight sizes
    party_weight_sizes.push_back(linear_reg_model.weight_size);
    for (int i = 0; i < party.party_num; i++) {
      if (i != party.party_id) {
        std::string recv_weight_size;
        party.recv_long_message(i, recv_weight_size);
        party_weight_sizes.push_back(std::stoi(recv_weight_size));
      }
    }
    // then broadcast the vector
    std::string party_weight_sizes_str;
    serialize_int_array(party_weight_sizes, party_weight_sizes_str);
    for (int i = 0; i < party.party_num; i++) {
      if (i != party.party_id) {
        party.send_long_message(i, party_weight_sizes_str);
      }
    }
  } else {
    // first send the weight size to active party
    party.send_long_message(ACTIVE_PARTY_ID, std::to_string(linear_reg_model.weight_size));
    // then receive and deserialize the party_weight_sizes array
    std::string recv_party_weight_sizes_str;
    party.recv_long_message(ACTIVE_PARTY_ID, recv_party_weight_sizes_str);
    deserialize_int_array(party_weight_sizes, recv_party_weight_sizes_str);
  }

  log_info("[compute_l1_regularized_grad]: finish aggregate and broadcast weight sizes");

  // step 2
  int global_weight_size = std::accumulate(party_weight_sizes.begin(), party_weight_sizes.end(), 0);
  auto* global_weights = new EncodedNumber[global_weight_size];
  if (party.party_type == falcon::ACTIVE_PARTY) {
    // first append its own local weights
    int global_idx = 0;
    for (int j = 0; j < linear_reg_model.weight_size; j++) {
      global_weights[global_idx] = linear_reg_model.local_weights[j];
      global_idx++;
    }
    for (int i = 0; i < party.party_num; i++) {
      if (i != party.party_id) {
        int recv_weight_size = party_weight_sizes[i];
        auto* recv_local_weights = new EncodedNumber[recv_weight_size];
        std::string recv_local_weights_str;
        party.recv_long_message(i, recv_local_weights_str);
        deserialize_encoded_number_array(recv_local_weights,
                                         recv_weight_size, recv_local_weights_str);
        for (int k = 0; k < recv_weight_size; k++) {
          global_weights[global_idx] = recv_local_weights[k];
          global_idx++;
        }
        delete [] recv_local_weights;
      }
    }
  } else {
    // serialize local weights and send to active party
    std::string local_weights_str;
    serialize_encoded_number_array(linear_reg_model.local_weights,
                                   linear_reg_model.weight_size, local_weights_str);
    party.send_long_message(ACTIVE_PARTY_ID, local_weights_str);
  }
  // active party broadcast the global weights vector
  party.broadcast_encoded_number_array(global_weights, global_weight_size, ACTIVE_PARTY_ID);

  log_info("[compute_l1_regularized_grad]: finish aggregate and broadcast global weights");

  // step 3
  std::vector<double> global_weights_shares;
  int phe_precision = abs(global_weights[0].getter_exponent());
  party.ciphers_to_secret_shares(global_weights, global_weights_shares,
                                 global_weight_size, ACTIVE_PARTY_ID, phe_precision);

  log_info("[compute_l1_regularized_grad]: finish convert to secret shares");

  // step 4
  // the spdz_linear_regression_computation will do the extracting sign with *(-1) operation
  std::promise<std::vector<double>> promise_values;
  std::future<std::vector<double>> future_values =
      promise_values.get_future();
  std::thread spdz_thread(spdz_linear_regression_computation,
                          party.party_num,
                          party.party_id,
                          party.executor_mpc_ports,
                          SPDZ_PLAYER_PATH,
                          party.host_names,
                          global_weights_shares,
                          global_weight_size,
                          &promise_values);

  std::vector<double> global_regularized_sign_shares = future_values.get();
  // main thread wait spdz_thread to finish
  spdz_thread.join();

  log_info("[compute_l1_regularized_grad]: finish connect to spdz parties and receive result shares");

  // step 5
  auto* global_regularized_grad = new EncodedNumber[global_weight_size];
  party.secret_shares_to_ciphers(global_regularized_grad, global_regularized_sign_shares,
                                 global_weight_size, ACTIVE_PARTY_ID, phe_precision);

  log_info("[compute_l1_regularized_grad]: finish convert regularized gradient shares to ciphers");

  // step 6
  djcs_t_public_key* phe_pub_key = djcs_t_init_public_key();
  party.getter_phe_pub_key(phe_pub_key);
  EncodedNumber regularization_hyper_param;
  regularization_hyper_param.set_double(phe_pub_key->n[0],
                                        alpha, 2 * PHE_FIXED_POINT_PRECISION);
  for (int j = 0; j < global_weight_size; j++) {
    djcs_t_aux_ep_mul(phe_pub_key, global_regularized_grad[j],
                      global_regularized_grad[j], regularization_hyper_param);
  }
  // assign to local regularized gradients
  int start_idx = 0;
  for (int i = 0; i < party.party_id; i++) {
    start_idx += party_weight_sizes[i];
  }
  for (int j = 0; j < linear_reg_model.weight_size; j++) {
    regularized_gradients[j] = global_regularized_grad[start_idx];
    start_idx++;
  }

  log_info("[compute_l1_regularized_grad]: finish assign to local regularized gradients");

  delete [] global_weights;
  delete [] global_regularized_grad;
  djcs_t_free_public_key(phe_pub_key);
}

void LinearRegressionBuilder::update_encrypted_weights(Party &party, EncodedNumber *encrypted_gradients) const {
  djcs_t_public_key* phe_pub_key = djcs_t_init_public_key();
  party.getter_phe_pub_key(phe_pub_key);
  // update the j-th weight in local_weight vector
  // need to make sure that the exponents of inner_product
  // and local weights are the same
  for (int j = 0; j < linear_reg_model.weight_size; j++) {
    djcs_t_aux_ee_add(phe_pub_key,
                      linear_reg_model.local_weights[j],
                      linear_reg_model.local_weights[j],
                      encrypted_gradients[j]);
  }
  djcs_t_free_public_key(phe_pub_key);
}

void spdz_linear_regression_computation(int party_num,
                                        int party_id,
                                        std::vector<int> mpc_port_bases,
                                        const std::string& mpc_player_path,
                                        std::vector<std::string> party_host_names,
                                        const std::vector<double>& global_weights_shares,
                                        int global_weight_size,
                                        std::promise<std::vector<double>> *regularized_grad_shares) {
  // Here put the whole setup socket code together, as using a function call
  // would result in a problem when deleting the created sockets
  // setup connections from this party to each spdz party socket
  std::vector<ssl_socket*> mpc_sockets(party_num);
  vector<int> plain_sockets(party_num);
  // ssl_ctx ctx(mpc_player_path, "C" + to_string(party_id));
  ssl_ctx ctx("C" + to_string(party_id));
  // std::cout << "correct init ctx" << std::endl;
  ssl_service io_service;
  octetStream specification;
  log_info("begin connect to spdz parties");
  log_info("party_num = " + std::to_string(party_num));
  for (int i = 0; i < party_num; i++)
  {
    log_info("[spdz_linear_regression_computation]: "
             "base:" + std::to_string(mpc_port_bases[i])
             + ", mpc_port_bases[i] + i: " + std::to_string(mpc_port_bases[i] + i));

    set_up_client_socket(plain_sockets[i], party_host_names[i].c_str(), mpc_port_bases[i] + i);
    send(plain_sockets[i], (octet*) &party_id, sizeof(int));
    mpc_sockets[i] = new ssl_socket(io_service, ctx, plain_sockets[i],
                                    "P" + to_string(i), "C" + to_string(party_id), true);
    if (i == 0){
      // receive gfp prime
      specification.Receive(mpc_sockets[0]);
    }
    LOG(INFO) << "Set up socket connections for " << i << "-th spdz party succeed,"
                                                          " sockets = " << mpc_sockets[i] << ", port_num = " << mpc_port_bases[i] + i << ".";
  }
  log_info("Finish setup socket connections to spdz engines.");
  int type = specification.get<int>();
  switch (type)
  {
    case 'p':
    {
      gfp::init_field(specification.get<bigint>());
      LOG(INFO) << "Using prime " << gfp::pr();
      break;
    }
    default:
      LOG(ERROR) << "Type " << type << " not implemented";
      exit(1);
  }
  log_info("Finish initializing gfp field.");
  // std::cout << "Finish initializing gfp field." << std::endl;
  // std::cout << "batch aggregation size = " << batch_aggregation_shares.size() << std::endl;

  // send data to spdz parties
  if (party_id == ACTIVE_PARTY_ID) {
    // send the current array size to the mpc parties
    std::vector<int> array_size_vec;
    array_size_vec.push_back(global_weight_size);
    send_public_values(array_size_vec, mpc_sockets, party_num);
  }

  // all the parties send private shares
  for (int i = 0; i < global_weights_shares.size(); i++) {
    vector<double> x;
    x.push_back(global_weights_shares[i]);
    send_private_inputs(x, mpc_sockets, party_num);
  }
  // send_private_inputs(batch_aggregation_shares,mpc_sockets, party_num);
  std::vector<double> return_values = receive_result(mpc_sockets, party_num, global_weight_size);
  regularized_grad_shares->set_value(return_values);

  for (int i = 0; i < party_num; i++) {
    close_client_socket(plain_sockets[i]);
  }

  // free memory and close mpc_sockets
  for (int i = 0; i < party_num; i++) {
    delete mpc_sockets[i];
    mpc_sockets[i] = nullptr;
  }
}

void LinearRegressionBuilder::train(Party party) {
  /// The training stage consists of the following steps
  /// 1. each party init encrypted local weights, "local_weights" = [w1], [w2], ...[wn]
  /// 2. iterative computation (see LogisticRegressionBuilder::train)
  ///   2.1 random select batch idx
  ///   2.2 compute batch_phe_aggregation on the batch samples [w_j * x_ij] for j = {1,...d}
  ///   2.3 compute gradient
  ///     2.3.1 if L2 regularization, do not need spdz computation,
  ///             gradient = {(1/B) \sum_{i=1}^{B} {([y_i] - [w_j * x_ij]) * 2 * x_ij}} + 2 * alpha * [w_j]
  ///     2.3.2 if L1 regularization, need spdz computation to assist
  ///             gradient = {(1/B) \sum_{i=1}^{B} {([y_i] - [w_j * x_ij]) * 2 * x_ij}} + \alpha, if w_j > 0
  ///             gradient = {(1/B) \sum_{i=1}^{B} {([y_i] - [w_j * x_ij]) * 2 * x_ij}} - \alpha, if w_j < 0
  ///           convert the [w_j] to secret shares, and return the original or negative \alpha
  ///             (need to ensure that the precision is matched during the whole computation)
  ///   2.4 use the computed gradient to update the encrypted weights
  /// 3. when the training finished, decrypt the encrypted weights if necessary

  log_info("************* Training Start *************");
  const clock_t training_start_time = clock();

  if (optimizer != "sgd") {
    log_error("The " + optimizer + " optimizer does not supported");
    exit(1);
  }

  // step 1: init encrypted local weights
  // (here use 3 * precision for consistence in the following)
  log_info("Init encrypted local weights");
  int encrypted_weights_precision = 3 * PHE_FIXED_POINT_PRECISION;
  int plaintext_samples_precision = PHE_FIXED_POINT_PRECISION;
  init_encrypted_random_numbers(party, linear_reg_model.weight_size,
                                linear_reg_model.local_weights,
                                encrypted_weights_precision);

  // record training data ids in data_indexes for iteratively batch selection
  std::vector<int> train_data_indexes;
  for (int i = 0; i < training_data.size(); i++) {
    train_data_indexes.push_back(i);
  }

  // required by spdz connector and mpc computation
  bigint::init_thread();

  // step 2: iteratively computation
  for (int iter = 0; iter < max_iteration; iter++) {
    log_info("-------- Iteration " + std::to_string(iter) + " --------");
    const clock_t iter_start_time = clock();

    // select batch_index
    std::vector< int> batch_indexes = select_batch_idx(party, batch_size, train_data_indexes);
    log_info("-------- Iteration " + std::to_string(iter) + ", select_batch_idx success --------");
    // get training data with selected batch_index
    std::vector<std::vector<double> > batch_samples;
    for (int index : batch_indexes) {
      batch_samples.push_back(training_data[index]);
    }

    // encode the training data
    int cur_sample_size = (int) batch_samples.size();
    auto** encoded_batch_samples = new EncodedNumber*[cur_sample_size];
    for (int i = 0; i < cur_sample_size; i++) {
      encoded_batch_samples[i] = new EncodedNumber[linear_reg_model.weight_size];
    }
    linear_reg_model.encode_samples(party, batch_samples, encoded_batch_samples);
    log_info("-------- Iteration " + std::to_string(iter) + ", encode training data success --------");

    // compute predicted label
    auto *predicted_labels = new EncodedNumber[batch_indexes.size()];
    // 4 * fixed precision
    // weight * xj => encrypted_weights_precision + plaintext_samples_precision
    int encrypted_batch_aggregation_precision = encrypted_weights_precision + plaintext_samples_precision;
    linear_reg_model.forward_computation(
        party,
        cur_sample_size,
        encoded_batch_samples,
        encrypted_batch_aggregation_precision,
        predicted_labels);
    log_info("-------- Iteration " + std::to_string(iter) + ", forward computation success --------");

    // note: from here should be different from the logistic regression logic
    // update encrypted local weights
    auto encrypted_gradients = new EncodedNumber[linear_reg_model.weight_size];
    int update_precision = encrypted_batch_aggregation_precision;
    backward_computation(
        party,
        batch_samples,
        predicted_labels,
        batch_indexes,
        update_precision,
        encrypted_gradients);

    log_info("-------- Iteration " + std::to_string(iter) + ", backward computation success --------");
    update_encrypted_weights(party, encrypted_gradients);
    log_info("-------- Iteration " + std::to_string(iter) + ", update_encrypted_weights computation success --------");

    const clock_t iter_finish_time = clock();
    double iter_consumed_time =
        double(iter_finish_time - iter_start_time) / CLOCKS_PER_SEC;
    log_info("-------- The " + std::to_string(iter) + "-th "
                                                      "iteration consumed time = " + std::to_string(iter_consumed_time));
    delete [] predicted_labels;
    delete [] encrypted_gradients;
    for (int i = 0; i < cur_sample_size; i++) {
      delete [] encoded_batch_samples[i];
    }
    delete [] encoded_batch_samples;
  }

  const clock_t training_finish_time = clock();
  double training_consumed_time =
      double(training_finish_time - training_start_time) / CLOCKS_PER_SEC;
  log_info("Training time = " + std::to_string(training_consumed_time));
  log_info("************* Training Finished *************");
}

void LinearRegressionBuilder::distributed_train(const Party &party, const Worker &worker) {

}

void LinearRegressionBuilder::eval(Party party, falcon::DatasetType eval_type, const std::string &report_save_path) {
  std::string dataset_str = (eval_type == falcon::TRAIN ? "training dataset" : "testing dataset");
  log_info("************* Evaluation on " + dataset_str + " Start *************");
  const clock_t testing_start_time = clock();

  /// the testing workflow is as follows:
  ///     step 1: init test data
  ///     step 2: the parties call the model.predict function to compute predicted labels
  ///     step 3: active party aggregates and call collaborative decryption
  ///     step 4: active party computes mse metrics

  // retrieve phe pub key and phe random
  djcs_t_public_key* phe_pub_key = djcs_t_init_public_key();
  party.getter_phe_pub_key(phe_pub_key);

  // step 1: init test data
  int dataset_size =
      (eval_type == falcon::TRAIN) ? training_data.size() : testing_data.size();
  std::vector<std::vector<double>> cur_test_dataset =
      (eval_type == falcon::TRAIN) ? training_data : testing_data;

  // step 2: every party do the prediction, since all examples are required to
  // computed, there is no need communications of data index between different parties
  auto* predicted_labels = new EncodedNumber[dataset_size];
  linear_reg_model.predict(party, cur_test_dataset, predicted_labels);

  // step 3: active party aggregates and call collaborative decryption
  auto* decrypted_labels = new EncodedNumber[dataset_size];
  party.collaborative_decrypt(predicted_labels,
                              decrypted_labels,
                              dataset_size,
                              ACTIVE_PARTY_ID);

  // std::cout << "Print predicted class" << std::endl;

  // step 4: active party computes the logistic function
  // and compare the clf metrics
  if (party.party_type == falcon::ACTIVE_PARTY) {
//    mean_squared_error(decrypted_labels, dataset_size, eval_type, report_save_path);
  }

  // free memory
  djcs_t_free_public_key(phe_pub_key);
  delete [] predicted_labels;
  delete [] decrypted_labels;

  const clock_t testing_finish_time = clock();
  double testing_consumed_time =
      double(testing_finish_time - testing_start_time) / CLOCKS_PER_SEC;
  log_info("Evaluation time = " + std::to_string(testing_consumed_time));
  log_info("************* Evaluation on " + dataset_str + " Finished *************");

}

void LinearRegressionBuilder::eval_predictions_and_save(EncodedNumber *decrypted_labels,
                                                        int sample_number,
                                                        falcon::DatasetType eval_type,
                                                        const std::string &report_save_path) {
  std::string dataset_str = (eval_type == falcon::TRAIN ? "training dataset" : "testing dataset");
  // assume we only support mse, but rmse, msle, rmsle, mae, maxe are also supported
  // we print these metrics together for better debug
  if (metric != "mse") {
    log_error("The " + metric + " metric is not supported");
    exit(1);
  }

  // Regression Metrics for performance evaluation
  RegressionMetrics reg_metrics;
  // decode decrypted labels
  std::vector<double> decoded_predicted_labels;
  for (int i = 0; i < sample_number; i++) {
    double decode_pred;
    decrypted_labels[i].decode(decode_pred);
    decoded_predicted_labels.push_back(decode_pred);
  }
  if (eval_type == falcon::TRAIN) {
    reg_metrics.compute_metrics(decoded_predicted_labels, training_labels);
  }
  if (eval_type == falcon::TEST) {
    reg_metrics.compute_metrics(decoded_predicted_labels, testing_labels);
  }

  // write the results to report
  std::ofstream outfile;
  if (!report_save_path.empty()) {
    outfile.open(report_save_path, std::ios_base::app);
    if (outfile) {
      outfile << "******** Evaluation Report on " << dataset_str
              << " ********\n";
    }
  }
  log_info("Regression Report on " + dataset_str + " is: ");
  reg_metrics.regression_report(outfile);
  outfile.close();
}


// initializes the LinearRegressionBuilder instance
// and run .train() .eval() methods, then save model
void train_linear_regression(
    Party* party,
    const std::string& params_str,
    const std::string& model_save_file,
    const std::string& model_report_file,
    int is_distributed_train, Worker* worker) {
  log_info("Run train_logistic_regression");
  log_info("is_distributed_train = " + std::to_string(is_distributed_train));

  LinearRegressionParams params;
  deserialize_lir_params(params, params_str);
  log_linear_regression_params(params);

  // split train test data for party and populate the vectors
  std::vector<std::vector<double>> training_data;
  std::vector<std::vector<double>> testing_data;
  std::vector<double> training_labels;
  std::vector<double> testing_labels;

  // if not distributed train, then the party split the data
  // otherwise, the party/worker receive the data and phe keys from ps
  if (is_distributed_train == 0) {
    double split_percentage = SPLIT_TRAIN_TEST_RATIO;
    split_dataset(party, params.fit_bias, training_data, testing_data,
                  training_labels, testing_labels, split_percentage);
  } else {
    // TODO: add distributed train implementation
  }

  // weight size is different if fit_bias is true on active party
  int weight_size = party->getter_feature_num();
  double training_accuracy = 0.0;
  double testing_accuracy = 0.0;
  LinearRegressionBuilder linear_reg_builder(params,
                                             weight_size,
                                             training_data,
                                             testing_data,
                                             training_labels,
                                             testing_labels,
                                             training_accuracy,
                                             testing_accuracy);

  if (is_distributed_train == 0) {
    linear_reg_builder.train(*party);
    linear_reg_builder.eval(*party, falcon::TRAIN, model_report_file);
    linear_reg_builder.eval(*party, falcon::TEST, model_report_file);
    // save model and report
    auto* model_weights = new EncodedNumber[weight_size];
    std::string pb_lr_model_string;
    serialize_lr_model(linear_reg_builder.linear_reg_model, pb_lr_model_string);
    save_pb_model_string(pb_lr_model_string, model_save_file);
    // save_lr_model(log_reg_model_builder.log_reg_model, model_save_file);
    // save_training_report(log_reg_model.getter_training_accuracy(),
    //    log_reg_model.getter_testing_accuracy(),
    //    model_report_file);
    delete[] model_weights;
  } else {
    // TODO: add distributed train implementation
  }
}