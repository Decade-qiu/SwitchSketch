import mmh3
import collections
import random
import decimal
import math
from matplotlib import pyplot as plt
import sys

def Query(item,id,FP1,FP2,FP3):
    if len(Level[3][id]) > 0:
        for index,value in enumerate(Level[3][id]):
            if value != 0:
                if value[0] == FP3:
                    return [index,3]
    if len(Level[2][id]) > 0:
        for index, value in enumerate(Level[2][id]):
            if value != 0:
                if value[0] == FP2:
                    return [index, 2]
    if len(Level[1][id]) > 0:
        for index,value in enumerate(Level[1][id]):
            if value != 0:
                if value[0] == FP1:
                    return [index,1]
    return 0

def Levelup(k):
    if k == 2:
        for index,item in enumerate(Level[3][bucket_index1]):
            if item == 0:
                Level[3][bucket_index1][index] = [FP_level3, 2 ** counter_bit_level2]
                return True
        if len(Level[1][bucket_index1]) >= 3:
            Clear(Level[1][bucket_index1], 3)
            Level[3][bucket_index1].append([FP_level3, 2 ** counter_bit_level2])
            count_levelup[1] += 1
            return True
        elif len(Level[2][bucket_index1]) >= 3:
            Clear(Level[2][bucket_index1], 3)
            Level[3][bucket_index1].append([FP_level3, 2 ** counter_bit_level2])
            Level[3][bucket_index1].append(0)
            count_levelup[2] += 1
            return True
        return False

    elif k == 1:
        for index,item in enumerate(Level[2][bucket_index1]):
            if item == 0:
                Level[2][bucket_index1][index] = [FP_level2, 2 ** counter_bit_level1]
                return True
        if len(Level[1][bucket_index1]) >= 3:
            Clear(Level[1][bucket_index1], 3)
            Level[2][bucket_index1].append([FP_level2, 2 ** counter_bit_level1])
            Level[2][bucket_index1].append(0)
            count_levelup[0] += 1
            return True
        return False

def Clear(lst,clear_num):
    for times in range(clear_num):
        Delete_Min(lst)

def Delete_Min(lst):
    min_num = sys.maxsize
    min_pos = -1
    for i in range(len(lst)):
        if lst[i] != 0:
            if lst[i][1] < min_num:
                min_num = lst[i][1]
                min_pos = i
        else:
            lst.pop(i)
            return lst

    lst.pop(min_pos)
    return lst

def Exponential_Decay(lst,k):
    if len(lst) == 0:
        return False
    min_num = sys.maxsize
    min_pos = -1
    for i in range(len(lst)):
        if lst[i] != 0:
            if isinstance(lst[i][1],int):
                if lst[i][1] < min_num:
                    min_num = lst[i][1]
                    min_pos = i
    if min_num == sys.maxsize:
        gailv_minus = 0
    else:
        gailv_minus = 1 / b ** math.log2(min_num)

    if random.random() < gailv_minus:
        if isinstance(lst[min_pos][1], int):
            lst[min_pos][1] -= 1
            if lst[min_pos][1] < 2**k:
                if k == counter_bit_level1:
                    lst[min_pos][0] = FP_level2
                elif k == counter_bit_level2:
                    lst[min_pos][0] = FP_level3
                lst[min_pos][1] = 2**k
                return True
    return False

def read_data(file_path):
    # Constants
    KEY_SIZE = 4  # Assuming the KEY_SIZE is 13 bytes (equivalent to C++ uint8_t array)
    TEMP2_SIZE = 9  # Assuming 9 bytes for temp2 in the original code
    # Store keys
    keys = []
    # Store actual flow sizes
    actual_flow_sizes = collections.defaultdict(int)
    
    # Open the binary file
    with open(file_path, 'rb') as fin:
        count = 0
        while True:
            # Read KEY_SIZE bytes
            key = fin.read(KEY_SIZE)
            if len(key) != KEY_SIZE:
                break  # End of file or unexpected size, stop reading
            
            # Append the key
            keys.append(key)
            
            # Update the flow size in the dictionary
            actual_flow_sizes[key] += 1
            count += 1
            
            # Read TEMP2_SIZE bytes (but we don't do anything with it, similar to temp2 in C++)
            temp2 = fin.read(TEMP2_SIZE)
            if len(temp2) != TEMP2_SIZE:
                break  # Break if we can't read the expected number of bytes
            
            # Log progress every 5 million keys
            if count % 5000000 == 0:
                print(f"Successfully read in {file_path}, {count} items")
        
        print(f"Successfully read in {file_path}, {count} items")
    
    return keys

def read_data(file_path):
    # Constants
    KEY_SIZE = 4  # Assuming the KEY_SIZE is 13 bytes (equivalent to C++ uint8_t array)
    TEMP2_SIZE = 9  # Assuming 9 bytes for temp2 in the original code
    # Store keys
    keys = []
    # Store actual flow sizes
    actual_flow_sizes = collections.defaultdict(int)
    
    # Open the binary file
    with open(file_path, 'rb') as fin:
        count = 0
        while True:
            # Read KEY_SIZE bytes
            key = fin.read(KEY_SIZE)
            if len(key) != KEY_SIZE:
                break  # End of file or unexpected size, stop reading
            
            # Append the key
            keys.append(key)
            
            # Update the flow size in the dictionary
            actual_flow_sizes[key] += 1
            count += 1
            
            # Read TEMP2_SIZE bytes (but we don't do anything with it, similar to temp2 in C++)
            temp2 = fin.read(TEMP2_SIZE)
            if len(temp2) != TEMP2_SIZE:
                break  # Break if we can't read the expected number of bytes
            
            # Log progress every 5 million keys
            if count % 5000000 == 0:
                print(f"Successfully read in {file_path}, {count} items")
                # break
        
        print(f"Successfully read in {file_path}, {count} items")
    
    return keys

if __name__ == '__main__':
    # read file
    # df = utils.File_Util.read_single_file(49, True, unique_tag=False)
    # F = df['tag1']
    F = read_data('../../C++/data/20190117-130000-new.dat')
    print('The numbers of packets:{}'.format(len(F)))
    l = collections.Counter(F)
    print(l.most_common(1))

    for _T in [750, 1500, 3000, 6000]:
        upper = _T * 4 / 3
        lst_PR = []
        lst_RR = []
        temp_memo = [50]
        if _T == 750:
            temp_memo = [50, 100, 150, 200, 250, 300]
        for _memorysize in temp_memo:
            memorysize = _memorysize
            alpha = 0.5
            bottom = upper * alpha
            T = (upper + bottom) / 2
            topK = 1024
            d = 8
            count_levelup = [0, 0, 0]
            b = 1.08
            memorysize = memorysize * 128/(128+24)

            counter_bit_level1 = 8
            fp_bit_level1 = 8

            counter_bit_level2 = 12
            fp_bit_level2 = 12

            counter_bit_level3 = 20
            fp_bit_level3 = 16

            gamma = 4

            # 单个bucket的bit
            W = (counter_bit_level1+fp_bit_level1)*d

            cell_num = int(W / counter_bit_level1 / 2)
            BUCKET_NUM = int(memorysize*1024*8/W)
            print('Memory Size:{}KB'.format(memorysize))
            print('d:{}'.format(d))
            print('T:{}'.format(T))
            print('alpha:{}'.format(alpha))
            print('bottom:{},top:{}'.format(bottom, upper))
            # print('The number of buckets:{}'.format(BUCKET_NUM))
            # print('The number of initial level-1 cell in each bucket:{}'.format(cell_num))

            count1 = 0
            count2 = 0
            count3 = 0
            count4 = 0
            Level1 = [[0 for j in range(cell_num)] for i in range(BUCKET_NUM)]
            Level2 = [[] for i in range(BUCKET_NUM)]
            Level3 = [[] for i in range(BUCKET_NUM)]
            Level = [0,Level1,Level2,Level3]

            set_HH = set()
            # SEED = [0,1,2,3,4,5]
            SEED = [2132, 315, 1651, 3165, 4651]

            for item in F:
                bucket_index1 = mmh3.hash(item, SEED[0]) % BUCKET_NUM
                FP_level1 = mmh3.hash(item,SEED[1]) % (2 ** fp_bit_level1)
                FP_level2 = mmh3.hash(item,SEED[3]) % (2 ** fp_bit_level2)
                FP_level3 = mmh3.hash(item,SEED[4]) % (2 ** fp_bit_level3)
                query_result = Query(item, bucket_index1, FP_level1, FP_level2, FP_level3)
                if query_result != 0:
                    query_index = query_result[0]
                    query_level = query_result[1]
                    if query_level == 3:
                        if Level[3][bucket_index1][query_index][1] == T:
                            set_HH.add(item)
                        Level[3][bucket_index1][query_index][1] += 1
                    elif query_level == 2:
                        if T < 4096 and Level[2][bucket_index1][query_index][1] == T:
                            set_HH.add(item)

                        if Level[2][bucket_index1][query_index][1] < 2**counter_bit_level2-1:
                            Level[2][bucket_index1][query_index][1] += 1
                        else:
                            query_FP = Level[2][bucket_index1][query_index][0]
                            if Levelup(2):
                                for index, item in enumerate(Level[2][bucket_index1]):
                                    if item != 0:
                                        if item[0] == query_FP:
                                            Level[2][bucket_index1][index] = 0
                            else:
                                count1 += 1
                                if Exponential_Decay(Level[3][bucket_index1], counter_bit_level2):
                                    count2 += 1
                                    Level[2][bucket_index1][query_index] = 0

                    elif query_level == 1:
                        if Level[1][bucket_index1][query_index][1] < 2 ** counter_bit_level1 - 1:
                            Level[1][bucket_index1][query_index][1] += 1
                        else:
                            query_FP = Level[1][bucket_index1][query_index][0]
                            if Levelup(1):
                                for index,item in enumerate(Level[1][bucket_index1]):
                                    if item != 0:
                                        if item[0] == query_FP:
                                            Level[1][bucket_index1][index] = 0
                            else:
                                count3 += 1
                                if Exponential_Decay(Level[2][bucket_index1], counter_bit_level1):
                                    count4 += 1
                                    Level[1][bucket_index1][query_index] = 0
                else:
                    noEmpty = True
                    for index, item in enumerate(Level[1][bucket_index1]):
                        if item == 0:
                            Level[1][bucket_index1][index] = [FP_level1, 1]
                            noEmpty = False
                            break
                    if noEmpty:
                        min_num_level1 = sys.maxsize
                        min_pos_level1 = -1
                        for i in range(len(Level[1][bucket_index1])):
                            if Level[1][bucket_index1] != 0:
                                if Level[1][bucket_index1][i][1] < min_num_level1:
                                    min_num_level1 = Level[1][bucket_index1][i][1]
                                    min_pos_level1 = i

                        if min_num_level1 > 9000:
                            gailv_minus = 0
                        else:
                            gailv_minus = 1/(b**(math.log2(min_num_level1)))

                        if random.random() < gailv_minus:
                            Level[1][bucket_index1][min_pos_level1][1] -= 1

                            if Level[1][bucket_index1][min_pos_level1][1] == 0:
                                Level[1][bucket_index1][min_pos_level1][0] = FP_level1
                                Level[1][bucket_index1][min_pos_level1][1] = 1

            l1 = l.most_common(topK)
            count = 0

            result_overT = {}
            result_all = {}

            real = {}

            real_less_than_bottom = {}
            real_larger_than_upper = {}

            for i in l.items():
                if i[1] > T:
                    real.update({i[0]: i[1]})
                if i[1] < bottom:
                    real_less_than_bottom.update({i[0]: i[1]})
                if i[1] > upper:
                    real_larger_than_upper.update({i[0]: i[1]})

            for key in set_HH:
                ID1 = mmh3.hash(key, SEED[0]) % BUCKET_NUM

                FP_l1 = mmh3.hash(key, SEED[1]) % (2 ** fp_bit_level1)
                FP_l2 = mmh3.hash(key, SEED[3]) % (2 ** fp_bit_level2)
                FP_l3 = mmh3.hash(key, SEED[4]) % (2 ** fp_bit_level3)
                query_result1 = Query(key, ID1, FP_l1, FP_l2, FP_l3)

                value_result = 0

                if query_result1 != 0:
                    query_index = query_result1[0]
                    query_level = query_result1[1]
                    if query_level == 3:
                        value_result += Level[3][ID1][query_index][1]
                    elif query_level == 2:
                        value_result += Level[2][ID1][query_index][1]
                    else:
                        value_result += Level[1][ID1][query_index][1]

                result_overT.update({key: value_result})

            aae_overT = 0
            aae_all = 0
            are_all = 0
            are_overT = 0

            count_bottom = 0
            count_upper_all = len(real_larger_than_upper)

            lst_sta = []

            for key in set_HH:
                if key in real:
                    count += 1
                if l[key] < bottom:
                    count_bottom += 1
                if key in real_larger_than_upper:
                    del real_larger_than_upper[key]

                aae_overT += abs(result_overT[key] - l[key])
                are_overT += abs(result_overT[key] - l[key]) / l[key]

            count_upper_mis = len(real_larger_than_upper)

            dic = sorted(result_overT.items(), key=lambda x: x[1], reverse=True)[:topK]
            lst_overT = sorted(result_overT.items(), key=lambda x: x[1], reverse=True)

            error = {}
            count_topk = 0
            dic_l1 = dict(l1)
            aae_topk = 0
            are_topk = 0

            for item in dic:
                aae_topk += abs(l[item[0]] - item[1])
                are_topk += abs(l[item[0]] - item[1]) / l[item[0]]
                if item[0] in dic_l1:
                    count_topk += 1
                    del dic_l1[item[0]]

            FPR = count_bottom / len(real_less_than_bottom)
            FNR = count_upper_mis / count_upper_all
            print('Top-theta aae:{},Top-theta are:{}'.format(aae_topk / topK, are_topk / topK))
            print('OverT aae:{},OverT are:{}'.format(aae_overT / len(result_overT), are_overT / len(result_overT)))
            print(count_bottom, len(real_less_than_bottom), count_upper_mis, count_upper_all)
            print('FPR:{},FNR:{}'.format(FPR, FNR))
            print(count, len(real), len(result_overT))
            print('Find top-{}:{}'.format(topK, count_topk))
            print('Threshold={},PR:{}'.format(T, count / len(result_overT)))
            print('Threshold={},RR:{}'.format(T, count / len(real)))
            lst_PR.append(count / len(result_overT))
            lst_RR.append(count / len(real))
            print('PR', lst_PR)
            print('RR', lst_RR)

            PR = count / len(result_overT)
            RR = count / len(real)

            if _T == 750 and _memorysize == 150:
                with open('./result/FNR.txt', 'a') as f:
                    f.write(f"DHS\n{FNR}\n")
                with open('./result/FPR.txt', 'a') as f:
                    f.write(f"DHS\n{FPR}\n")

            if _T == 750:
                with open('./result/F2_memory.txt', 'a') as f:
                    if _memorysize == 50:
                        f.write(f"DHS\n{5*PR*RR/(4*PR+RR)}")
                    else:
                        f.write(f" {5*PR*RR/(4*PR+RR)}")
                    if _memorysize == 300:
                        f.write('\n')

            if _memorysize == 50:
                with open('./result/F2_t.txt', 'a') as f:
                    if _T == 750:
                        f.write(f"DHS\n{5*PR*RR/(4*PR+RR)}")
                    else:
                        f.write(f" {5*PR*RR/(4*PR+RR)}")
                    if _T == 6000:
                        f.write('\n')