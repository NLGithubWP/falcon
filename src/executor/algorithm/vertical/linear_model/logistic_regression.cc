//
// Created by wuyuncheng on 14/11/20.
//

#include <falcon/algorithm/vertical/linear_model/logistic_regression.h>
#include <falcon/utils/pb_converter/common_converter.h>

#include <ctime>
#include <random>
#include <thread>
#include <future>

#include <glog/logging.h>
#include <Networking/ssl_sockets.h>
#include <falcon/operator/mpc/spdz_connector.h>

LogisticRegression::LogisticRegression() {}

LogisticRegression::LogisticRegression(int m_batch_size,
    int m_max_iteration,
    float m_converge_threshold,
    bool m_with_regularization,
    float m_alpha,
    float m_learning_rate,
    float m_decay,
    int m_weight_size,
    std::string m_penalty,
    std::string m_optimizer,
    std::string m_multi_class,
    std::string m_metric,
    std::vector<std::vector<float> > m_training_data,
    std::vector<std::vector<float> > m_testing_data,
    std::vector<float> m_training_labels,
    std::vector<float> m_testing_labels,
    float m_training_accuracy,
    float m_testing_accuracy) : Model(std::move(m_training_data),
        std::move(m_testing_data),
        std::move(m_training_labels),
        std::move(m_testing_labels),
        m_training_accuracy,
        m_testing_accuracy) {
  batch_size = m_batch_size;
  max_iteration = m_max_iteration;
  converge_threshold = m_converge_threshold;
  with_regularization = m_with_regularization;
  alpha = m_alpha;
  learning_rate = m_learning_rate;
  decay = m_decay;
  weight_size = m_weight_size;
  penalty = std::move(m_penalty);
  optimizer = std::move(m_optimizer);
  multi_class = std::move(m_multi_class);
  metric = std::move(m_metric);
  local_weights = new EncodedNumber[weight_size];
}

LogisticRegression::~LogisticRegression() {
  delete [] local_weights;
}

void LogisticRegression::init_encrypted_weights(const Party &party, int precision) {
  LOG(INFO) << "Init encrypted local weights.";
  djcs_t_public_key* phe_pub_key = djcs_t_init_public_key();
  hcs_random* phe_random = hcs_init_random();

  party.getter_phe_pub_key(phe_pub_key);
  party.getter_phe_random(phe_random);

  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_real_distribution<float> dist(0.0, 1.0);
  // srand(static_cast<unsigned> (time(nullptr)));
  for (int i = 0; i < weight_size; i++) {
    // generate random float values within (0, 1],
    // init fixed point EncodedNumber,
    // and encrypt with public key
    float v = dist(mt);
    // float v = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    EncodedNumber t;
    t.set_float(phe_pub_key->n[0], v, precision);
    djcs_t_aux_encrypt(phe_pub_key, phe_random, local_weights[i], t);
  }

  djcs_t_free_public_key(phe_pub_key);
  hcs_free_random(phe_random);
}

std::vector<int> LogisticRegression::select_batch_idx(const Party &party,
    std::vector<int> data_indexes) {
  std::vector<int> batch_indexes;
  // if active party, randomly select batch indexes and send to other parties
  // if passive parties, receive batch indexes and return
  if (party.party_type == falcon::ACTIVE_PARTY) {
    std::random_device rd;
    std::default_random_engine rng(rd());
    std::shuffle(std::begin(data_indexes), std::end(data_indexes), rng);
    for (int i = 0; i < batch_size; i++) {
      batch_indexes.push_back(data_indexes[i]);
    }
    // send to other parties
    for (int i = 0; i < party.party_num; i++) {
      if (i != party.party_id) {
        std::string batch_indexes_str;
        serialize_int_array(batch_indexes, batch_indexes_str);
        party.send_long_message(i, batch_indexes_str);
      }
    }
  } else {
    std::string recv_batch_indexes_str;
    party.recv_long_message(0, recv_batch_indexes_str);
    deserialize_int_array(batch_indexes, recv_batch_indexes_str);
  }
  return batch_indexes;
}

void LogisticRegression::compute_batch_phe_aggregation(const Party &party,
    std::vector<int> batch_indexes,
    int precision,
    EncodedNumber *batch_phe_aggregation) {
  // retrieve phe pub key and phe random
  djcs_t_public_key* phe_pub_key = djcs_t_init_public_key();
  hcs_random* phe_random = hcs_init_random();
  party.getter_phe_pub_key(phe_pub_key);
  party.getter_phe_random(phe_random);

  // retrieve batch samples and encode (notice to use cur_batch_size
  // instead of default batch size to avoid unexpected batch)
  int cur_batch_size = batch_indexes.size();
  std::vector< std::vector<float> > batch_samples;
  for (int i = 0; i < cur_batch_size; i++) {
    batch_samples.push_back(training_data[batch_indexes[i]]);
  }
  EncodedNumber** encoded_batch_samples = new EncodedNumber*[cur_batch_size];
  for (int i = 0; i < cur_batch_size; i++) {
    encoded_batch_samples[i] = new EncodedNumber[weight_size];
  }
  for (int i = 0; i < cur_batch_size; i++) {
    for (int j = 0; j < weight_size; j++) {
      encoded_batch_samples[i][j].set_float(phe_pub_key->n[0],
          batch_samples[i][j], precision);
    }
  }

  // compute local homomorphic aggregation
  EncodedNumber* local_batch_phe_aggregation = new EncodedNumber[cur_batch_size];
  djcs_t_aux_matrix_mult(phe_pub_key, phe_random, local_batch_phe_aggregation,
      local_weights, encoded_batch_samples, cur_batch_size, weight_size);

  // every party sends the local aggregation to the active party
  if (party.party_type == falcon::ACTIVE_PARTY) {
    // copy self local aggregation results
    for (int i = 0; i < cur_batch_size; i++) {
      batch_phe_aggregation[i] = local_batch_phe_aggregation[i];
    }

    // receive serialized string from other parties
    // deserialize and sum to batch_phe_aggregation
    for (int id = 0; id < party.party_num; id++) {
      if (id != party.party_id) {
        EncodedNumber* recv_batch_phe_aggregation = new EncodedNumber[cur_batch_size];
        std::string recv_local_aggregation_str;
        party.recv_long_message(id, recv_local_aggregation_str);
        deserialize_encoded_number_array(recv_batch_phe_aggregation,
            cur_batch_size, recv_local_aggregation_str);
        // homomorphic addition of the received aggregations
        for (int i = 0; i < cur_batch_size; i++) {
          djcs_t_aux_ee_add(phe_pub_key,batch_phe_aggregation[i],
              batch_phe_aggregation[i], recv_batch_phe_aggregation[i]);
        }
        delete [] recv_batch_phe_aggregation;
      }
    }

    // serialize and send the batch_phe_aggregation to other parties
    std::string global_aggregation_str;
    serialize_encoded_number_array(batch_phe_aggregation,
        cur_batch_size, global_aggregation_str);
    for (int id = 0; id < party.party_num; id++) {
      if (id != party.party_id) {
        party.send_long_message(id, global_aggregation_str);
      }
    }
  } else {
    // serialize local batch aggregation and send to active party
    std::string local_aggregation_str;
    serialize_encoded_number_array(local_batch_phe_aggregation,
        cur_batch_size, local_aggregation_str);
    party.send_long_message(0, local_aggregation_str);

    // receive the global batch aggregation from the active party
    std::string recv_global_aggregation_str;
    party.recv_long_message(0, recv_global_aggregation_str);
    deserialize_encoded_number_array(batch_phe_aggregation,
        cur_batch_size, recv_global_aggregation_str);
  }

  djcs_t_free_public_key(phe_pub_key);
  hcs_free_random(phe_random);
  for (int i = 0; i < cur_batch_size; i++) {
    delete [] encoded_batch_samples[i];
  }
  delete [] encoded_batch_samples;
  delete [] local_batch_phe_aggregation;
}

void LogisticRegression::update_encrypted_weights(Party& party,
    std::vector<float> batch_loss_shares,
    std::vector<float> truncated_weight_shares,
    std::vector<int> batch_indexes,
    int precision) {
  // retrieve phe pub key and phe random
  djcs_t_public_key *phe_pub_key = djcs_t_init_public_key();
  hcs_random *phe_random = hcs_init_random();
  party.getter_phe_pub_key(phe_pub_key);
  party.getter_phe_random(phe_random);

  // convert batch loss shares back to encrypted losses
  int cur_batch_size = batch_indexes.size();
  EncodedNumber *encrypted_batch_losses = new EncodedNumber[cur_batch_size];
  party.secret_shares_to_ciphers(encrypted_batch_losses,
                                 batch_loss_shares,
                                 cur_batch_size,
                                 0,
                                 precision);

  std::vector<std::vector<float> > batch_samples;
  for (int i = 0; i < cur_batch_size; i++) {
    batch_samples.push_back(training_data[batch_indexes[i]]);
  }
  EncodedNumber **encoded_batch_samples = new EncodedNumber *[cur_batch_size];
  for (int i = 0; i < cur_batch_size; i++) {
    encoded_batch_samples[i] = new EncodedNumber[weight_size];
  }

  // update formula: [w_j]=[w_j]-lr*(1/|B|){\sum_{i=1}^{|B|} [loss_i]*x_{ij}} + reg?
  // lr*(1/|B|) is the same for all sample values, thus can be initialized
  float lr_batch = learning_rate / cur_batch_size;
  for (int i = 0; i < cur_batch_size; i++) {
    for (int j = 0; j < weight_size; j++) {
      encoded_batch_samples[i][j].set_float(phe_pub_key->n[0],
          0 - lr_batch * batch_samples[i][j], precision);
    }
  }

  // if not with_regularization, no need to convert truncated weights;
  // otherwise, need to convert truncated weights to ciphers for the update
  if (!with_regularization) {
    // update each local weight j
    for (int j = 0; j < weight_size; j++) {
      EncodedNumber inner_product;
      EncodedNumber* batch_feature_j = new EncodedNumber[cur_batch_size];
      for (int i = 0; i < cur_batch_size; i++) {
        batch_feature_j[i] = encoded_batch_samples[i][j];
      }
      djcs_t_aux_inner_product(phe_pub_key,
          phe_random,
          inner_product,
          encrypted_batch_losses,
          batch_feature_j,
          cur_batch_size);
      // need to make sure that the exponents of inner_product and local weights are same
      djcs_t_aux_ee_add(phe_pub_key, local_weights[j], local_weights[j], inner_product);
      delete [] batch_feature_j;
    }
  } else {
    //TODO: handle the update formula when using regularization, currently only l2
  }

  djcs_t_free_public_key(phe_pub_key);
  hcs_free_random(phe_random);
  delete [] encrypted_batch_losses;
  for (int i = 0; i < cur_batch_size; i++) {
    delete [] encoded_batch_samples[i];
  }
  delete [] encoded_batch_samples;
}

void LogisticRegression::train(Party party) {
  LOG(INFO) << "************* Training Start *************";
  const clock_t training_start_time = clock();

  /// The training stage consists of the following steps
  /// 1. init encrypted local weights
  /// 2. iterative computation
  ///     2.1 randomly select a batch of indexes for current iteration
  ///     2.2 homomorphic aggregation on the batch samples
  ///     2.3 convert the batch ciphertexts to secret shares
  ///     2.4 connect to spdz parties for mpc computations
  ///     2.5 receive secret shared results from spdz party and aggregate
  ///     2.6 update encrypted local weights carefully
  /// 3. decrypt weights ciphertext

  // step 1: init encrypted local weights (here use 2 * precision for consistence in the following)
  int encrypted_weights_precision = 2 * PHE_FIXED_POINT_PRECISION;
  int plaintext_samples_precision = PHE_FIXED_POINT_PRECISION;
  init_encrypted_weights(party, encrypted_weights_precision);

  // record training data ids in data_indexes for iteratively batch selection
  std::vector<int> data_indexes;
  for (int i = 0; i < training_data.size(); i++) {
    data_indexes.push_back(i);
  }

  // required by spdz connector and mpc computation
  bigint::init_thread();

  // step 2: iteratively computation
  for (int iter = 0; iter < max_iteration; iter++) {
    LOG(INFO) << "-------- Iteration " << iter << " --------";
    std::cout << "-------- Iteration " << iter << " --------" << std::endl;
    const clock_t iter_start_time = clock();

    // step 2.1: randomly select a batch of samples
    std::vector<int> batch_indexes = select_batch_idx(party, data_indexes);
    int cur_batch_size = batch_indexes.size();

    std::cout << "step 2.1 success" << std::endl;

    // step 2.2: homomorphic batch aggregation (the precision should be 3 * prec now)
    EncodedNumber* encrypted_batch_aggregation = new EncodedNumber[cur_batch_size];
    compute_batch_phe_aggregation(party,
        batch_indexes,
        plaintext_samples_precision,
        encrypted_batch_aggregation);

    std::cout << "step 2.2 success" << std::endl;

    // step 2.3: convert the encrypted batch aggregation into secret shares
    int encrypted_batch_aggregation_precision = encrypted_weights_precision + plaintext_samples_precision;
    std::vector<float> batch_aggregation_shares;
    party.ciphers_to_secret_shares(encrypted_batch_aggregation,
        batch_aggregation_shares,
        cur_batch_size,
        0,
        encrypted_batch_aggregation_precision);

    std::cout << "step 2.3 success" << std::endl;

    std::promise<std::vector<float>> promise_values;
    std::future<std::vector<float>> future_values = promise_values.get_future();
    std::thread spdz_thread(spdz_logistic_function_computation,
        party.party_num,
        party.party_id,
        SPDZ_PORT_BASE,
        SPDZ_PLAYER_PATH,
        party.host_names,
        batch_aggregation_shares,
        cur_batch_size,
        &promise_values);
    std::vector<float> batch_loss_shares = future_values.get();
    spdz_thread.join();

    std::cout << "step 2.4 success" << std::endl;

    // step 2.6: update encrypted local weights
    // TODO: currently does not support with_regularization
    std::vector<float> truncated_weights_shares;
    // need to make sure that update_precision * 2 = encrypted_weights_precision
    int update_precision = PHE_FIXED_POINT_PRECISION;
    update_encrypted_weights(party,
        batch_loss_shares,
        truncated_weights_shares,
        batch_indexes,
        update_precision);

    std::cout << "step 2.5 success" << std::endl;

    delete [] encrypted_batch_aggregation;

    const clock_t iter_finish_time = clock();
    float iter_consumed_time = float(iter_finish_time - iter_start_time) / CLOCKS_PER_SEC;
    LOG(INFO) << "-------- The " << iter << "-th iteration consumed time = " << iter_consumed_time << " --------";
    std::cout << "-------- The " << iter << "-th iteration consumed time = " << iter_consumed_time << " --------" << std::endl;
  }

  const clock_t training_finish_time = clock();
  float training_consumed_time = float(training_finish_time - training_start_time) / CLOCKS_PER_SEC;
  LOG(INFO) << "Training time = " << training_consumed_time;
  LOG(INFO) << "************* Training Finished *************";
  google::FlushLogFiles(google::INFO);
}

void spdz_logistic_function_computation(int party_num,
    int party_id,
    int mpc_port_base,
    std::string mpc_player_path,
    std::vector<std::string> party_host_names,
    std::vector<float> batch_aggregation_shares,
    int cur_batch_size,
    std::promise<std::vector<float>> *batch_loss_shares) {
  std::vector<ssl_socket*> mpc_sockets(party_num);
  //  setup_sockets(party_num,
  //                party_id,
  //                std::move(mpc_player_path),
  //                std::move(party_host_names),
  //                mpc_port_base,
  //                mpc_sockets);

  // Here put the whole setup socket code together, as using a function call
  // would result in a problem when deleting the created sockets
  // setup connections from this party to each spdz party socket
  vector<int> plain_sockets(party_num);
  ssl_ctx ctx(mpc_player_path, "C" + to_string(party_id));
  ssl_service io_service;
  octetStream specification;
  for (int i = 0; i < party_num; i++)
  {
    set_up_client_socket(plain_sockets[i], party_host_names[i].c_str(), mpc_port_base + i);
    send(plain_sockets[i], (octet*) &party_id, sizeof(int));
    mpc_sockets[i] = new ssl_socket(io_service, ctx, plain_sockets[i],
                                "P" + to_string(i), "C" + to_string(party_id), true);
    if (i == 0){
      // receive gfp prime
      specification.Receive(mpc_sockets[0]);
    }
    LOG(INFO) << "Set up socket connections for " << i << "-th spdz party succeed,"
                                                          " sockets = " << mpc_sockets[i] << ", port_num = " << mpc_port_base + i << ".";
  }
  LOG(INFO) << "Finish setup socket connections to spdz engines.";
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
  LOG(INFO) << "Finish initializing gfp field.";
  send_private_inputs(batch_aggregation_shares,mpc_sockets, party_num);
  std::vector<float> return_values = receive_result(mpc_sockets, party_num, cur_batch_size);
  batch_loss_shares->set_value(return_values);

  // free memory and close mpc_sockets
  for (int i = 0; i < party_num; i++) {
    delete mpc_sockets[i];
    mpc_sockets[i] = nullptr;
  }
}

void train_logistic_regression(Party party, std::string params) {

  LOG(INFO) << "Run the example logistic regression train";
  std::cout << "Run the example logistic regression train" << std::endl;

  // TODO: Parse the params and match with the LogisticRegression parameters
  // currently for testing
  int batch_size = 32;
  int max_iteration = 5;
  float converge_threshold = 1e-3;
  bool with_regularization = false;
  float alpha = 0.1;
  float learning_rate = 0.05;
  float decay = 1.0;
  std::string penalty = "l2";
  std::string optimizer = "sgd";
  std::string multi_class = "ovr";
  std::string metric = "acc";
  int weight_size = party.getter_feature_num();
  float training_accuracy = 0.0;
  float testing_accuracy = 0.0;

  std::vector< std::vector<float> > training_data;
  std::vector< std::vector<float> > testing_data;
  std::vector<float> training_labels;
  std::vector<float> testing_labels;
  float split_percentage = 0.8;
  party.split_train_test_data(split_percentage,
      training_data,
      testing_data,
      training_labels,
      testing_labels);

  LOG(INFO) << "Init logistic regression model";
  std::cout << "Init logistic regression model" << std::endl;

  LogisticRegression model(batch_size,
      max_iteration,
      converge_threshold,
      with_regularization,
      alpha,
      learning_rate,
      decay,
      weight_size,
      penalty,
      optimizer,
      multi_class,
      metric,
      training_data,
      testing_data,
      training_labels,
      testing_labels,
      training_accuracy,
      testing_accuracy);

  LOG(INFO) << "Init model success";
  std::cout << "Init model success" << std::endl;

  model.train(party);
}