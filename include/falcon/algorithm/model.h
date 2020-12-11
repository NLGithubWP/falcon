//
// Created by wuyuncheng on 14/11/20.
//

#ifndef FALCON_SRC_EXECUTOR_ALGORITHM_MODEL_H_
#define FALCON_SRC_EXECUTOR_ALGORITHM_MODEL_H_

#include <vector>

class Model {
 protected:
  // training data (without label column)
  std::vector< std::vector<float> > training_data;
  // testing data (without label column)
  std::vector< std::vector<float> > testing_data;
  // labels of training samples
  std::vector<float> training_labels;
  // labels of testing samples
  std::vector<float> testing_labels;
  // training accuracy
  float training_accuracy;
  // testing accuracy
  float testing_accuracy;

 public:
  /**
   * default constructor
   */
  Model();

  /**
   * model constructor
   *
   * @param m_training_data
   * @param m_testing_data
   * @param m_training_labels
   * @param m_testing_labels
   * @param m_training_accuracy
   * @param m_testing_accuracy
   */
  Model(std::vector< std::vector<float> > m_training_data,
      std::vector< std::vector<float> > m_testing_data,
      std::vector<float> m_training_labels,
      std::vector<float> m_testing_labels,
      float m_training_accuracy = 0.0,
      float m_testing_accuracy = 0.0);

  /**
   * destructor
   */
  ~Model();

  /** set training data */
  void setter_training_data(std::vector< std::vector<float> > s_training_data) {
    training_data = s_training_data;
  }

  /** set testing data */
  void setter_testing_data(std::vector< std::vector<float> > s_testing_data) {
    testing_data = s_testing_data;
  }

  /** set training labels */
  void setter_training_labels(std::vector<float> s_training_labels) {
    training_labels = s_training_labels;
  }

  /** set testing labels */
  void setter_testing_labels(std::vector<float> s_testing_labels) {
    testing_labels = s_testing_labels;
  }

  /** set training accuracy */
  void setter_training_accuracy(float s_training_accuracy) {
    training_accuracy = s_training_accuracy;
  }

  /** set testing accuracy */
  void setter_testing_accuracy(float s_testing_accuracy) {
    testing_accuracy = s_testing_accuracy;
  }

  /** get training data */
  std::vector< std::vector<float> > getter_training_data() { return training_data; }

  /** get testing data */
  std::vector< std::vector<float> > getter_testing_data() { return testing_data; }

  /** get training labels */
  std::vector<float> getter_training_labels() { return training_labels; }

  /** get testing labels */
  std::vector<float> getter_testing_labels() { return testing_labels; }

  /** get training accuracy */
  float getter_training_accuracy() { return training_accuracy; }

  /** get testing accuracy */
  float getter_testing_accuracy() { return testing_accuracy; }
};

#endif //FALCON_SRC_EXECUTOR_ALGORITHM_MODEL_H_