//
// Created by wuyuncheng on 14/11/20.
//

#ifndef FALCON_SRC_EXECUTOR_ALGORITHM_VERTICAL_LINEAR_MODEL_LOGISTIC_REGRESSION_H_
#define FALCON_SRC_EXECUTOR_ALGORITHM_VERTICAL_LINEAR_MODEL_LOGISTIC_REGRESSION_H_

#include <falcon/operator/phe/fixed_point_encoder.h>
#include <falcon/algorithm/model.h>
#include <falcon/party/party.h>

#include <vector>
#include <string>
#include <thread>
#include <future>

struct LogisticRegressionParams {
  // size of mini-batch in each iteration
  int batch_size;
  // maximum number of iterations for training
  int max_iteration;
  // tolerance of convergence
  float converge_threshold;
  // whether use regularization or not
  bool with_regularization;
  // regularization parameter
  float alpha;
  // learning rate for parameter updating
  float learning_rate;
  // decay rate for learning rate, following lr = lr0 / (1 + decay*t), t is #iteration
  float decay;
  // penalty method used, 'l1' or 'l2', default l2, currently support 'l2'
  std::string penalty;
  // optimization method, default 'sgd', currently support 'sgd'
  std::string optimizer;
  // strategy for handling multi-class classification, default 'ovr', currently support 'ovr'
  std::string multi_class;
  // evaluation metric for training and testing, 'acc', 'auc', or 'ks', currently support 'acc'
  std::string metric;
  // differential privacy (DP) budget, 0 denotes not use DP
  float dp_budget;
};

class LogisticRegression : public Model {
 public:
  // size of mini-batch in each iteration
  int batch_size;
  // maximum number of iterations for training
  int max_iteration;
  // tolerance of convergence
  float converge_threshold;
  // whether use regularization or not
  bool with_regularization;
  // regularization parameter
  float alpha;
  // learning rate for parameter updating
  float learning_rate;
  // decay rate for learning rate, following lr = lr0 / (1 + decay*t), t is #iteration
  float decay;
  // penalty method used, 'l1' or 'l2', default l2, currently support 'l2'
  std::string penalty;
  // optimization method, default 'sgd', currently support 'sgd'
  std::string optimizer;
  // strategy for handling multi-class classification, default 'ovr', currently support 'ovr'
  std::string multi_class;
  // evaluation metric for training and testing, 'acc', 'auc', or 'ks', currently support 'acc'
  std::string metric;

 private:
  // number of weights in the model
  int weight_size;
  // model weights vector, encrypted values during training, size equals to weight_size
  EncodedNumber* local_weights;

 public:
  /** default constructor */
  LogisticRegression();

  /**
   * logistic regression constructor
   *
   * @param m_batch_size: mini-batch size
   * @param m_max_iteration: maximum number of iteration
   * @param m_converge_threshold: tolerance of convergence
   * @param m_with_regularization: use regularization or not
   * @param m_alpha: regularization parameter
   * @param m_learning_rate: learning rate
   * @param m_decay: decay rate
   * @param m_weight_size: number of weights
   * @param m_penalty: penalty method
   * @param m_optimizer: optimization method
   * @param m_multi_class: strategy
   * @param m_metric: evaluation metric
   * @param m_training_data: training data
   * @param m_testing_data: testing data
   * @param m_training_labels: training labels
   * @param m_testing_labels: testing labels
   * @param m_training_accuracy: training accuracy
   * @param m_testing_accuracy: testing accuracy
   */
  LogisticRegression(int m_batch_size,
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
      std::vector< std::vector<float> > m_training_data,
      std::vector< std::vector<float> > m_testing_data,
      std::vector<float> m_training_labels,
      std::vector<float> m_testing_labels,
      float m_training_accuracy = 0.0,
      float m_testing_accuracy = 0.0);

  /** destructor */
  ~LogisticRegression();

  /**
   * initialize encrypted local weights
   *
   * @param party: initialized party object
   * @param precision: precision for big integer representation EncodedNumber
   */
  void init_encrypted_weights(const Party& party, int precision = PHE_FIXED_POINT_PRECISION);

  /**
   * select batch indexes for each iteration
   *
   * @param party: initialized party object
   * @param data_indexes: the original training data indexes
   * @return
   */
  std::vector<int> select_batch_idx(const Party& party, std::vector<int> data_indexes);

  /**
   * compute phe aggregation for a batch of samples
   *
   * @param party: initialized party object
   * @param batch_indexes: selected batch indexes
   * @param precision: the fixed point precision of encoded plaintext samples
   * @param batch_aggregation: returned phe aggregation for the batch
   */
  void compute_batch_phe_aggregation(const Party& party,
      std::vector<int> batch_indexes,
      int precision,
      EncodedNumber *batch_phe_aggregation);

  /**
   * after receiving batch loss shares and truncated weight shares
   * from spdz parties, update the encrypted local weights
   *
   * @param party: initialized party object
   * @param batch_loss_shares: secret shares of batch losses
   * @param truncated_weight_shares: truncated global weights if with regularization
   * @param batch_indexes: selected batch indexes
   * @param precision: precision for the batch samples and shares
   */
  void update_encrypted_weights(Party& party,
      std::vector<float> batch_loss_shares,
      std::vector<float> truncated_weight_shares,
      std::vector<int> batch_indexes,
      int precision);

  /**
   * train a logistic regression model
   *
   * @param party: initialized party object
   */
  void train(Party party);
};

/**
 * spdz computation with thread
 *
 * @param party_num: number of parties
 * @param party_id: current party id
 * @param mpc_port_base: port base of the spdz parties
 * @param mpc_player_path: player data path of the spdz parties
 * @param party_host_names: spdz parties host names (ips)
 * @param batch_aggregation_shares: the batch shares
 * @param cur_batch_size: size of current batch
 * @param batch_loss_shares: promise structure of the loss shares
 */
void spdz_logistic_function_computation(int party_num,
    int party_id,
    int mpc_port_base,
    std::string mpc_player_path,
    std::vector<std::string> party_host_names,
    std::vector<float> batch_aggregation_shares,
    int cur_batch_size,
    std::promise<std::vector<float>> *batch_loss_shares);

void train_logistic_regression(Party party, std::string params);

#endif //FALCON_SRC_EXECUTOR_ALGORITHM_VERTICAL_LINEAR_MODEL_LOGISTIC_REGRESSION_H_