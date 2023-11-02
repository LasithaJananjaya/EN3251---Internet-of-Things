import pandas as pd
import numpy as np
from sklearn.model_selection import  GridSearchCV
from sklearn.svm import SVC
from sklearn.preprocessing import StandardScaler
from sklearn.metrics import accuracy_score
import joblib

# Load the training and testing data
train_data = pd.read_csv('wifi_data_train.csv')
test_data = pd.read_csv('wifi_data_test1.csv')

# Split the training and testing data into input and output sets
X_train = train_data.iloc[:, :-1]
y_train = train_data.iloc[:, -1]
X_test = test_data.iloc[:, :-1]
y_test = test_data.iloc[:, -1]

# Data preprocessing - Standardize the features
scaler = StandardScaler()
X_train = scaler.fit_transform(X_train)
X_test = scaler.transform(X_test)

param_grid = [
    {'kernel': ['linear'], 'C': [0.1, 1, 10, 100]},
    {'kernel': ['poly'],   'C': [0.1, 1, 10, 100], 'degree': [2, 3, 4], 'gamma': [0.01, 0.1, 1]},
    {'kernel': ['rbf'],    'C': [0.1, 1, 10, 100], 'gamma': [0.01, 0.1, 1]}
]

# Use Grid Search to find the best hyperparameters
grid_search = GridSearchCV(SVC(), param_grid , cv=5, scoring='accuracy')

#Training the entire model on the best model with found parameters
grid_search.fit(X_train, y_train)

#Displaying the
print("The best model is :")
print("Kernel = {} \nC = {}".format(grid_search.best_params_.get("kernel"),grid_search.best_params_.get("C") ))
if (grid_search.best_params_.get("kernel") == "poly"):
    print("Degree = {} \nGamma = {}".format(grid_search.best_params_.get('degree'), grid_search.best_params_.get('gamma')))
if (grid_search.best_params_.get("kernel") == "rbf"):
    print("Gamma = {}".format(grid_search.best_params_.get('gamma')))

# Train the best SVM model
print("Training the model")
grid_search.fit(X_train, y_train)

# Evaluate the performance of the trained model
print("Testing with test data")
y_pred = grid_search.predict(X_test)
accuracy = accuracy_score(y_test, y_pred)
print('Best Model - Accuracy: {:.3f}%'.format(accuracy*100))

# Save the best trained model
joblib.dump(grid_search, 'best_wifi_model.pkl')
