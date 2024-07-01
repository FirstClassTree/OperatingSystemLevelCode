import numpy as np
import matplotlib.pyplot as plt
from sklearn.datasets import fetch_openml
import numpy.random
from scipy.spatial import distance
import matplotlib.pyplot as plt

# Function that is the answer for a
def KNN(train_images, lables_vector, query_image, k):
    #calulating all eucliding distances
    distances = distance.cdist(train_images, query_image.reshape(1,-1),'euclidean')
    # getting the k with neareast ones indexs with a sort
    indexes = distances.flatten().argsort()[:k]
    #we now get the lables
    nearest_k_lables = lables_vector[indexes].astype(int)
    #we will now count label prevelence
    label_count = np.bincount(nearest_k_lables)
    #and get the max one
    return np.argmax(label_count)


def main():
    mnist = fetch_openml('mnist_784', as_frame=False)
    data = mnist['data']
    labels = mnist['target'].astype(int)
    idx = numpy.random.RandomState(0).choice(70000, 11000)
    train = data[idx[:1000], :].astype(int)
    train_labels = labels[idx[:1000]]
    test = data[idx[1000:], :].astype(int)
    test_labels = labels[idx[1000:]]
     
    #Answer for b part question
    predictions = []
    for image in test:
       prediction = KNN(train, train_labels, image, 10)
       predictions.append(prediction)

    accuracy = np.mean(predictions == test_labels)
    print(f"Accuracy: {accuracy * 100}%")
    
    
    # #Answer for c part question
    accuracy_list = []
    best_k = 1 
    best_accuracy = 0
    for k in range(1,101):
        predictions = []
        for image in test:
            prediction = KNN(train, train_labels, image, k)
            predictions.append(prediction)

        accuracy = np.mean(predictions == test_labels)
        accuracy_list.append(accuracy)
        if accuracy > best_accuracy:
            best_accuracy = accuracy
            best_k = k
    
    print(f"Best K: {best_k}")
    plt.plot(range(1,101),accuracy_list)
    plt.xlabel('k')
    plt.ylabel('Accuracy Percentage')
    plt.savefig('accuracy_plot.png')
    plt.show()

    #Answer for d part question
    dataset_sizes = range(100, 5100, 100)
    accuracies = []
    for dataset_size in dataset_sizes:
        train = data[idx[:dataset_size], :].astype(int)
        train_labels = labels[idx[:dataset_size]]
        test = data[idx[dataset_size:], :].astype(int)
        test_labels = labels[idx[dataset_size:]]

        predictions = []
        for image in test:
            prediction = KNN(train, train_labels, image, 1)  # k=1
            predictions.append(prediction)

        accuracy = np.mean(predictions == test_labels)
        accuracies.append(accuracy)

    plt.plot(dataset_sizes, accuracies)
    plt.xlabel('Dataset size')
    plt.ylabel('Accuracy')
    plt.title('Accuracy for different dataset sizes')
    plt.savefig('dataset_size_vs_accuracy.png')
    plt.show()

     






if __name__ == "__main__":
    main()