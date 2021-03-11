# Sample Dataset Used in Falcon Platform

## How Falcon Uses the Dataset

### Split-train-test

Executor split the train/test in `src/executor/party/party.cc`:
```c++
void Party::split_train_test_data(float split_percentage,
...
  // if the party is active party, shuffle the data and labels,
  // and send the shuffled indexes to passive parties for splitting accordingly
...
    // send the data_indexes to passive parties
```

The `src/executor/algorithm/vertical/linear_model/logistic_regression.cc` splits by a ratio of 80-20:

```cpp
  float split_percentage = 0.8;
  party.split_train_test_data(split_percentage,
      training_data,
      testing_data,
      training_labels,
      testing_labels);
```

## Logistic Regression: UCI Tele-marketing Bank Dataset

Here we introduce the logistic regression (LR) model, similar to the linear regression model.
In the vertical FL setting, the features are distributed among parties
while the label is held by only the active party. We use a hybrid of MPC and PHE 
to train the LR model.

More details on the LR model training and setup can be found at `src/executor/algorithm/vertical/linear_model/README.md`

### UCI Bank Marketing Data Set Details

URL of the data set at https://archive.ics.uci.edu/ml/datasets/Bank+Marketing

It is a dataset from a Portuguese bank, where they used machine learning for tele-marketing. The classification goal is a binary Yes (0) or No (1) of whether a client will subscribe a term deposit after the tele-marketing.

The bank marketing data contains 20 feature inputs, 10 are numeric features and 10 are categorical features.

### Data Preparation

The data set values are normalized with `tools/data_preparation/data_normalization.cc`, the output of the normalized is in `data/dataset/bank_marketing_data/bank.data.norm`

The training data consists of 3616 rows, while the testing data consists of 905 rows.


### Split Features for Vertical FL

The feature inputs (attributes) are split into clients, for vertial federated learning, using `tools/data_preparation/data_spliter.cc`

For example, the 0th row of the bank data:
```py
# csv schema:
"age";"job";"marital";"education";"default";"balance";"housing";"loan";"contact";"day";"month";"duration";"campaign";"pdays";"previous";"poutcome";"y"
# original csv data:
30;"unemployed";"married";"primary";"no";1787;"no";"no";"cellular";19;"oct";79;1;-1;0;"unknown";"no"
# original data preprocessed:
30,1,1,1,1,1787,1,1,1,19,10,79,1,-1,0,0,1
# original data preprocessed and normalized:
0.161777,0.090992,0.500250,0.333555,1.000000,0.068455,1.000000,1.000000,0.500250,0.600013,0.818198,0.024827,0.000020,0.000001,0.000040,0.000333,1.000000
```

Q: why does client-0 have 7 features, but client-1 and client-2 only have 5 features each?

A: **client-0 gets the LAST 7-column part of the row:**
```py
0.818198,0.024827,0.000020,0.000001,0.000040,0.000333,1.000000
```

**client-1 gets the middle 5-column part of the row:**
```py
0.068455,1.000000,1.000000,0.500250,0.600013
```

**client-2 gets the first 5-column part of the row:**
```py
0.161777,0.090992,0.500250,0.333555,1.000000
```


For **active party only (client-0)**, the last column is labels (Yes=0 or No=1)

- using seed=42, eval accuracy on training dataset is: 0.887721; testing dataset is: 0.872928;
- using seed=1, eval accuracy on training dataset is: 0.88385; testing dataset is 0.888398;
- generally  the evaluation accuracy should be around 87-90%

**NOTE! Because of imbalanced training leading to blind class prediction!**

_legend for confusion matrix: Prediction = rows, Ground Truth = cols_

when using seed=42, the actual confusion matrix is:
```
N_pred = 3616
Y_pred = 0
N_label = 3210
Y_label = 406
matched_num = 3210
	Confusion Matrix
		0 0 
		406 3210 

N_pred = 905
Y_pred = 0
N_label = 790
Y_label = 115
matched_num = 790
	Confusion Matrix
		0 0 
		115 790 
```
Therefore for seed=42, train acc = 3210/3616 = 0.887721, and test acc = 790/905 = 0.872928

and when seed=1, the actual confusion matrix is:
```
N_pred = 3616
Y_pred = 0
N_label = 3196
Y_label = 420
matched_num = 3196
	Confusion Matrix
		0 0 
		420 3196

N_pred = 905
Y_pred = 0
N_label = 804
Y_label = 101
matched_num = 804
	Confusion Matrix
		0 0 
		101 804
```
Therefore for seed=1, train acc = 3196/3616 = 0.883849, and test acc = 804/905 = 0.88839779

**So the logistic model is blinding predicting all No(1)!**


### Logistic Regression with just Client-0 (Active) Features

### Logistic Regression with Client-0 + Client-1 data
