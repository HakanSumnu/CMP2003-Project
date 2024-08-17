#include "IBCF.hpp"
#include <cmath>
#include <vector>
#include <sstream>
#include <string>
#include <fstream>
#include <iostream>
#include <memory.h>
#include <stdlib.h>
#include <time.h>

int IBCF::itemHashFunction(int item_id)
{
    return item_id;
}

int IBCF::userHashFunction(int user_id)
{
    return user_id;
}

int IBCF::userHashFunction(int user_id, int capacity)
{
    return user_id % capacity;
}

IBCF::IBCF()
{
    items = nullptr;
    users = nullptr;
    average_ratings_of_users = nullptr;
    euclidean_norms_of_items = nullptr;
    //item_similarity_table = nullptr;
    item_hashmap_size = 0;
    user_hashmap_size = 0;
}

IBCF::~IBCF()
{
    if (items != nullptr)
    {
        for (int i = 0; i < item_hashmap_size; ++i)
        {
            if (items[i].capacity != 0)
            {
                for (int j = 0; j < items[i].capacity; ++j)
                {
                    User_Node* tail_current = nullptr;
                    User_Node* current = items[i].user_hashmap[j];

                    while (current != nullptr)
                    {
                        tail_current = current;
                        current = current->next;
                        delete tail_current;
                    }
                }

                delete[] items[i].user_hashmap;
            }
        }

        delete[] items;
    }

    delete[] users;
    delete[] average_ratings_of_users;
    delete[] euclidean_norms_of_items;

    /*
    if (item_similarity_table != nullptr)
    {
        for (int i = 0; i < item_hashmap_size; ++i)
        {
            delete[] item_similarity_table[i];
        }

        delete[] item_similarity_table;
    }
    */
}

void IBCF::readData(const char* train_data_path)
{
    std::stringstream ss("");
    std::string data;

    std::ifstream in(train_data_path);
    if (!in.is_open())
    {
        std::cerr << "Train data cannot be opened." << std::endl;
        exit(-1);
    }
    std::getline(in, data); //For the first row UserID,ItemID,Rating

    while (in)
    {
        std::getline(in, data, ','); //UserID
        if (in.eof())
            break;
        ss << data;
        ss << '\n';
        std::getline(in, data, ','); //ItemID
        ss << data;
        ss << '\n';
        std::getline(in, data); //Rating
        ss << data;
        ss << '\n';
    }

    in.close();

    std::stringstream ss2(ss.str());
    
    int user_id;
    int item_id;
    double rating;

    //For finding maximmum value of user ids and item ids
    while (ss)
    {
        ss >> user_id;
        if (ss.eof())
            break;
        ss >> item_id;
        ss >> rating;

        if (user_id > user_hashmap_size)
        {
            user_hashmap_size = user_id;
        }

        if (item_id > item_hashmap_size)
        {
            item_hashmap_size = item_id;
        }
    }

    //item_hashmap_size = maximum_value_of_item_ids
    ++item_hashmap_size;
    //user_hashmap_size = maximum_value_of_user_ids
    ++user_hashmap_size;

    //Creation of items hashmap for storing ratings of users and number of users rated item i
    items = new Item[item_hashmap_size];
    memset(items, 0, sizeof(Item) * item_hashmap_size);
    //Creation of users hashmap for storing which user rated how many items
    users = new int[user_hashmap_size];
    memset(users, 0, sizeof(int) * user_hashmap_size);
    //Creation of average ratings of users hashmap for storing averages of ratings of users
    average_ratings_of_users = new double[user_hashmap_size];
    memset(average_ratings_of_users, 0, sizeof(double) * user_hashmap_size);
    //Creation of euclidean norms of items hashmap for storing euclidean norms(lengths) of ratings of items vector
    euclidean_norms_of_items = new double[item_hashmap_size];
    memset(euclidean_norms_of_items, 0, sizeof(double) * item_hashmap_size);

    ss = std::stringstream(ss2.str());

    while (true)
    {
        ss >> user_id;
        if (ss.eof())
            break;
        ss >> item_id;
        ss >> rating;

        //To find number of users that rated item i
        items[item_id].capacity += 1;
        //To find number of items that were rated by user i
        users[user_id] += 1;
        //To find sum of ratings of user i
        average_ratings_of_users[user_id] += rating;
    }

    for (int i = 0; i < item_hashmap_size; ++i)
    {
        if (items[i].capacity != 0)
        {
            items[i].user_hashmap = new User_Node*[items[i].capacity];
            for (int j = 0; j < items[i].capacity; ++j)
                items[i].user_hashmap[j] = nullptr;
        }
    }

    //Sum of ratings of user i divided by number of items that were rated by user i
    for (int i = 0; i < user_hashmap_size; ++i)
    {
        if (users[i] != 0)
        {
            average_ratings_of_users[i] /= users[i];
        }
    }

    while (true)
    {
        ss2 >> user_id;
        if (ss2.eof())
            break;
        ss2 >> item_id;
        ss2 >> rating;

        int user_index = userHashFunction(user_id, items[item_id].capacity);
        User_Node* new_node = new User_Node;
        new_node->user_id = user_id;
        new_node->rating = rating;
        new_node->next = items[item_id].user_hashmap[user_index];
        items[item_id].user_hashmap[user_index] = new_node;

        //To find sum of square of ratings of item vectors
        euclidean_norms_of_items[item_id] += (rating - average_ratings_of_users[user_id]) * (rating - average_ratings_of_users[user_id]);
    }

    //euclidean_norms_of_items[i] = (sum of square of ratings of item vectors)^(1/2)
    for (int i = 0; i < item_hashmap_size; ++i)
    {
        euclidean_norms_of_items[i] = sqrt(euclidean_norms_of_items[i]);
    }
}

void IBCF::printTop10()
{
    std::cout << "Top 10 users" << std::endl;
    std::cout << "User ids: Ratings" << std::endl;
    int maximums[10] { };
    int maximums_ids[10] { };
    for (int i = 0; i < user_hashmap_size; ++i)
    {
        for (int j = 0; j < 10; ++j)
        {
            if (maximums[j] < users[i])
            {
                for (int k = 9; k > j; --k)
                {
                    maximums[k] = maximums[k - 1];
                    maximums_ids[k] = maximums_ids[k - 1];
                }
                maximums[j] = users[i];
                maximums_ids[j] = i;
                break;
            }
        }
    }

    for (int i = 0; i < 10; ++i)
    {
        std::cout << maximums_ids[i] << ": " << maximums[i] << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Top 10 items" << std::endl;
    std::cout << "Item ids: Ratings" << std::endl;

    memset(maximums, 0, sizeof(int) * 10);

    for (int i = 0; i < item_hashmap_size; ++i)
    {
        for (int j = 0; j < 10; ++j)
        {
            if (maximums[j] < items[i].capacity)
            {
                for (int k = 9; k > j; --k)
                {
                    maximums[k] = maximums[k - 1];
                    maximums_ids[k] = maximums_ids[k - 1];
                }
                maximums[j] = items[i].capacity;
                maximums_ids[j] = i;
                break;
            }
        }
    }

    for (int i = 0; i < 10; ++i)
    {
        std::cout << maximums_ids[i] << ": " << maximums[i] << std::endl;
    }

    std::cout << std::endl;
}

void IBCF::predict(const char* test_data_path, const char* predicted_data_path, int k_neighbour)
{
    std::ifstream in(test_data_path);

    if (!in.is_open())
    {
        std::cerr << "Test data cannot be opened!" << std::endl;
        exit(-1);
    }

    std::string data;
    std::getline(in, data); //To eliminate first row ID,UserID,ItemID

    std::ofstream out(predicted_data_path);

    out << "ID,Predicted\n";

    int id;
    int user_id;
    int item_id;

    User_Node* current = nullptr;
    User_Node* current2 = nullptr;

    double numerator;
    double denumerator;
    double rating;
    double* max_k_similarity = new double[k_neighbour]{};
    double* k_ratings = new double[k_neighbour]{};

    while (true)
    {
        in >> id;
        if (in.eof())
            break;
        in.ignore();
        in >> user_id;
        in.ignore();
        in >> item_id;

        numerator = 0.0;
        denumerator = 0.0;
        current = nullptr;
        current2 = nullptr;
        memset(max_k_similarity, 0, sizeof(double) * k_neighbour);
        memset(k_ratings, 0, sizeof(double) * k_neighbour);

        for (int i = 0; i < item_hashmap_size; ++i)
        {
            //If there is no item at index i continue
            if (items[i].capacity == 0)
                continue;

            current = items[i].user_hashmap[userHashFunction(user_id, items[i].capacity)];

            //If searching user_id is bot in this item continue
            while (current != nullptr)
            {
                if (current->user_id == user_id)
                {
                    rating = current->rating - average_ratings_of_users[user_id];
                    break;
                }
                current = current->next;
            };

            if (current == nullptr)
                continue;

            //To find cosine similarity between two items
            double similarity = 0.0;

            if (items[item_id].capacity < items[i].capacity)
            {
                for (int j = 0; j < items[item_id].capacity; ++j)
                {
                    current = items[item_id].user_hashmap[j];

                    while (current != nullptr)
                    {
                        current2 = items[i].user_hashmap[userHashFunction(current->user_id, items[i].capacity)];
                        while (current2 != nullptr)
                        {
                            if (current->user_id == current2->user_id)
                            {
                                similarity += 
                                (current->rating - average_ratings_of_users[current->user_id]) * (current2->rating - average_ratings_of_users[current->user_id]);
                                break;
                            }
                            current2 = current2->next;
                        }
                        current = current->next;
                    }                    
                }
            }

            else
            {
                for (int j = 0; j < items[i].capacity; ++j)
                {
                    current = items[i].user_hashmap[j];

                    while (current != nullptr)
                    {
                        current2 = items[item_id].user_hashmap[userHashFunction(current->user_id, items[item_id].capacity)];
                        while (current2 != nullptr)
                        {
                            if (current->user_id == current2->user_id)
                            {
                                similarity += 
                                (current->rating - average_ratings_of_users[current->user_id]) * (current2->rating - average_ratings_of_users[current->user_id]);
                                break;
                            }
                            current2 = current2->next;
                        }
                        current = current->next;
                    }  
                }
            }

            similarity /= euclidean_norms_of_items[item_id] * euclidean_norms_of_items[i];

            for (int k = 0; k < k_neighbour; ++k)
            {
                if (abs(similarity) > abs(max_k_similarity[k]))
                {
                    //Shifting right elements which are have less similarity
                    for (int j = k_neighbour - 1; j > k; --j)
                    {
                        max_k_similarity[j] = max_k_similarity[j - 1];
                        k_ratings[j] = k_ratings[j - 1];
                    }
                    max_k_similarity[k] = similarity;
                    k_ratings[k] = rating;
                    break;
                }
            }
        }

        for (int j = 0; j < k_neighbour; ++j)
        {
            numerator += max_k_similarity[j] * k_ratings[j];
            denumerator += abs(max_k_similarity[j]);
        }

        double prediction = average_ratings_of_users[user_id] + (numerator / denumerator);
        if (std::isnan(prediction) == true)
        {
            std::cerr << "Prediction was nan" << std::endl;
            exit(-1);
        }

        if (abs(prediction) == INFINITY)
        {
            std::cerr << "Prediction was infinity" << std::endl;
            exit(-1);
        }

        out << id << ',' << prediction << '\n';
    }

    in.close();
    out.close();

    delete[] max_k_similarity;
    delete[] k_ratings;
}

void IBCF::test(int test_num, int k_neighbour, int random_seed)
{
    int random_user_id = 0;
    int random_item_id = 0;
    double true_rating = 0.0;
    double rmse = 0.0;
    int counter2 = test_num;
    User_Node* current = nullptr;
    User_Node* current2 = nullptr;

    double numerator = 0.0;
    double denumerator = 0.0;
    double rating = 0.0;
    double* max_k_similarity = new double[k_neighbour]{};
    double* k_ratings = new double[k_neighbour]{};

    std::vector<int> existed_item_ids;
    std::vector<int> existed_user_ids;

    srand(random_seed);

    for (int i = 0; i < test_num; ++i)
    {
        //Founding random user_id and item_id
        bool found = false;
        while (found == false)
        {
            found = true;
            while (true)
            {
                random_item_id = rand() % item_hashmap_size;
                if (items[random_item_id].user_hashmap != nullptr)
                {
                    existed_item_ids.push_back(random_item_id);
                    break;
                }
            };

            random_user_id = (rand() % (items[random_item_id].capacity)) + 1;
            for (int j = 0; j < existed_user_ids.size(); ++j)
            {
                if (existed_user_ids[j] == random_user_id && existed_item_ids[j] == random_item_id)
                {
                    found = false;
                    break;
                }
            }
            if (found == true)
            {
                existed_user_ids.push_back(random_user_id);
            }

            if (existed_user_ids.size() == 0)
            {
                existed_user_ids.push_back(random_user_id);
            }

            if (found == false)
                existed_item_ids.pop_back();
        }

        int counter = 0;
        found = false;

        for (int j = 0; j < items[random_item_id].capacity; ++j)
        {
            current = items[random_item_id].user_hashmap[j];
            while (current != nullptr)
            {
                ++counter;
                if (counter == random_user_id)
                {
                    random_user_id = current->user_id;
                    true_rating = current->rating;
                    found = true;
                    break;
                }
                current = current->next;
            }

            if (found == true)
                break;
        }

        numerator = 0.0;
        denumerator = 0.0;
        current = nullptr;
        current2 = nullptr;
        memset(max_k_similarity, 0, sizeof(double) * k_neighbour);
        memset(k_ratings, 0, sizeof(double) * k_neighbour);

        for (int j = 0; j < item_hashmap_size; ++j)
        {
            if (j == random_item_id)
                continue;
            
            //If there is no item at index i continue
            if (items[j].capacity == 0)
                continue;

            current = items[j].user_hashmap[userHashFunction(random_user_id, items[j].capacity)];

            //If searching user_id is bot in this item continue
            while (current != nullptr)
            {
                if (current->user_id == random_user_id)
                {
                    rating = current->rating - average_ratings_of_users[random_user_id];
                    break;
                }
                current = current->next;
            };

            if (current == nullptr)
                continue;

            //To find cosine similarity between two items
            double similarity = 0.0;

            if (items[random_item_id].capacity < items[j].capacity)
            {
                for (int k = 0; k < items[random_item_id].capacity; ++k)
                {
                    current = items[random_item_id].user_hashmap[k];

                    while (current != nullptr)
                    {
                        current2 = items[j].user_hashmap[userHashFunction(current->user_id, items[j].capacity)];
                        while (current2 != nullptr)
                        {
                            if (current->user_id == current2->user_id)
                            {
                                similarity += 
                                (current->rating - average_ratings_of_users[current->user_id]) * (current2->rating - average_ratings_of_users[current->user_id]);
                                break;
                            }
                            current2 = current2->next;
                        }
                        current = current->next;
                    }                    
                }
            }

            else
            {
                for (int k = 0; k < items[j].capacity; ++k)
                {
                    current = items[j].user_hashmap[k];

                    while (current != nullptr)
                    {
                        current2 = items[random_item_id].user_hashmap[userHashFunction(current->user_id, items[random_item_id].capacity)];
                        while (current2 != nullptr)
                        {
                            if (current->user_id == current2->user_id)
                            {
                                similarity += 
                                (current->rating - average_ratings_of_users[current->user_id]) * (current2->rating - average_ratings_of_users[current->user_id]);
                                break;
                            }
                            current2 = current2->next;
                        }
                        current = current->next;
                    }  
                }
            }

            similarity /= euclidean_norms_of_items[random_item_id] * euclidean_norms_of_items[j];

            for (int k = 0; k < k_neighbour; ++k)
            {
                if (abs(similarity) > max_k_similarity[k])
                {
                    //Shifting right elements which are have less similarity
                    for (int t = k_neighbour - 1; t > k; --t)
                    {
                        max_k_similarity[t] = max_k_similarity[t - 1];
                        k_ratings[t] = k_ratings[t - 1];
                    }
                    max_k_similarity[k] = similarity;
                    k_ratings[k] = rating;
                    break;
                }
            }
        }

        if (max_k_similarity[0] == 0.0)
        {
            --counter2;
            continue;
        }

        for (int k = 0; k < k_neighbour; ++k)
        {
            numerator += max_k_similarity[k] * k_ratings[k];
            denumerator += abs(max_k_similarity[k]);
        }

        double prediction = 
        (average_ratings_of_users[random_user_id] * users[random_user_id] - true_rating) / ((double)(users[random_user_id] - 1.0)) + (numerator / denumerator);
        if (std::isnan(prediction) == true)
        {
            std::cerr << "Prediction was nan" << std::endl;
            exit(-1);
        }

        if (abs(prediction) == INFINITY)
        {
            std::cerr << "Prediction was infinity" << std::endl;
            exit(-1);
        }

        rmse += (prediction - true_rating) * (prediction - true_rating);
    }

    rmse /= (double)counter2;
    rmse = (double)sqrt(rmse);

    std::cout << "Test finished" << std::endl;
    std::cout << "RMSE: " << rmse << std::endl;
}