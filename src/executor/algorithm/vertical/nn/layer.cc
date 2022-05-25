//
// Created by root on 5/21/22.
//

#include <falcon/algorithm/vertical/nn/layer.h>

Layer::Layer() {
  m_num_neurons = 0;
  m_num_inputs_per_neuron = 0;
  m_neurons.clear();
}

Layer::Layer(int num_neurons, int num_inputs_per_neuron,
             bool with_bias, const std::string &activation_func_str) {
  m_num_neurons = num_neurons;
  m_num_inputs_per_neuron = num_inputs_per_neuron;
  m_activation_func_str = activation_func_str;
  for (int i = 0; i < num_neurons; i++) {
    m_neurons.emplace_back(Neuron(num_inputs_per_neuron, with_bias));
  }
}

Layer::~Layer() {
  m_num_neurons = 0;
  m_num_inputs_per_neuron = 0;
  m_neurons.clear();
}