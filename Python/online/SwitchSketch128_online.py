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

def Get_C1(C3, C2):
    if W - meta - C3*(fp_bit_level3+counter_bit_level3)-C2*(fp_bit_level2+counter_bit_level2) < 0:
        return 99
    else:
        return int((W - meta - C3 * (fp_bit_level3 + counter_bit_level3) - C2 * (fp_bit_level2 + counter_bit_level2)) / (
                    fp_bit_level1 + counter_bit_level1))


def Adjust(C3_old,C2_old,C1_old,C3_new,C2_new,C1_new):
    #reconstruct a new l2
    if C3_new == C3_old and C2_new == C2_old + 1:
        Level[2][bucket_index1].append([FP_level2,(2**(counter_bit_level1))*2])
        lst_index[bucket_index1][1] += 1

        clear_num = -C1_new-C2_new-C3_new+C1_old+C2_old+C3_old


        # clear the original record
        for index, item in enumerate(Level[1][bucket_index1]):
            if item != 0:
                if item[0] == FP_level1:
                    Level[1][bucket_index1].pop(index)
                    break

        Clear_l1(Level[1][bucket_index1], clear_num)

    # reconstruct a new l3
    else:
        Level[3][bucket_index1].append([FP_level3,2**counter_bit_level2])
        clear_num_l1 = C1_old-C1_new
        clear_num_l2 = C2_old-C2_new
        #clear l1 cells only
        # none
        if clear_num_l2 == 0 and clear_num_l1 > 0:
            # lst_index[bucket_index1][0] += 1
            # Clear_l1(Level[1][bucket_index1], clear_num_l1)
            # #clear the original record
            # for index, item in enumerate(Level[2][bucket_index1]):
            #     if item != 0:
            #         if item[0] == FP_level2:
            #             Level[2][bucket_index1][index] = 0
            print("clear l1 cells only TAG")
            return

        #clear l2 cells only
        elif clear_num_l1 == 0 and clear_num_l2 > 0:
            lst_index[bucket_index1][0] += 1
            lst_index[bucket_index1][1] -= 1
            # clear the original record
            for index, item in enumerate(Level[2][bucket_index1]):
                if item != 0:
                    if item[0] == FP_level2:
                        Level[2][bucket_index1].pop(index)
                        break
            return

        # clear one l2 and clear one l1
        elif clear_num_l2 == 1 and clear_num_l1 == 1:
            # clear the original record
            for index, item in enumerate(Level[2][bucket_index1]):
                if item != 0:
                    if item[0] == FP_level2:
                        Level[2][bucket_index1].pop(index)
                        break
            Clear_l1(Level[1][bucket_index1], 1)
            lst_index[bucket_index1][0] += 1
            lst_index[bucket_index1][1] -= 1

        # clear two l2s and add one l1
        else:
            # clear the original record
            for index, item in enumerate(Level[2][bucket_index1]):
                if item != 0:
                    if item[0] == FP_level2:
                        Level[2][bucket_index1].pop(index)
                        break
            Clear_l2(Level[2][bucket_index1], 1)
            lst_index[bucket_index1][0] += 1
            lst_index[bucket_index1][1] -= 2
            Level[1][bucket_index1].append(0)


    return

def Switch(k, C3, C2):
    C1 = Get_C1(C3, C2)
    if k == 1:
        result = Get_C1(C3, C2 + 1)
        if result != 99:
            C1_new = result
            Adjust(C3,C2,C1,C3,C2+1,C1_new)
        #l2 exponential decay
        else:
            if len(Level[2][bucket_index1]) == 0:
                return
            min_num = sys.maxsize
            min_pos = -1
            for i in range(len(Level[2][bucket_index1])):
                if Level[2][bucket_index1][i] != 0:
                    if isinstance(Level[2][bucket_index1][i][1], int):
                        if Level[2][bucket_index1][i][1] < min_num:
                            min_num = Level[2][bucket_index1][i][1]
                            min_pos = i
            if min_num == sys.maxsize:
                gailv_minus = 0
            else:
                gailv_minus = 1 / b ** math.log2(min_num)

            if random.random() < gailv_minus:
                if isinstance(Level[2][bucket_index1][min_pos][1], int):
                    Level[2][bucket_index1][min_pos][1] -= 1
                    if Level[2][bucket_index1][min_pos][1] < 2 ** (counter_bit_level1+1):
                        Level[2][bucket_index1][min_pos][0] = FP_level2
                        Level[2][bucket_index1][min_pos][1] = 2 ** (counter_bit_level1+1)
                        for index, item in enumerate(Level[1][bucket_index1]):
                            if item != 0:
                                if item[0] == FP_level1 and item[1] == 2 ** counter_bit_level1 - 1:
                                    Level[1][bucket_index1][index] = 0

    else:
        flag = False
        for i in range(C2-1,-1,-1):
            result = Get_C1(C3 + 1, i)
            if result == 99:
                continue
            else:
                C1_new = result
                Adjust(C3, C2, C1, C3+1, i, C1_new)
                flag = True
                break
        # l3 exponential decay
        if not flag:
            if len(Level[3][bucket_index1]) == 0:
                return
            min_num = sys.maxsize
            min_pos = -1
            for i in range(len(Level[3][bucket_index1])):
                if Level[3][bucket_index1][i] != 0:
                    if isinstance(Level[3][bucket_index1][i][1], int):
                        if Level[3][bucket_index1][i][1] < min_num:
                            min_num = Level[3][bucket_index1][i][1]
                            min_pos = i
            if min_num == sys.maxsize:
                gailv_minus = 0
            else:
                gailv_minus = 1 / b ** math.log2(min_num)

            if random.random() < gailv_minus:
                if isinstance(Level[3][bucket_index1][min_pos][1], int):
                    Level[3][bucket_index1][min_pos][1] -= 1
                    if Level[3][bucket_index1][min_pos][1] < 2 ** (counter_bit_level2):
                        Level[3][bucket_index1][min_pos][0] = FP_level3
                        Level[3][bucket_index1][min_pos][1] = 2 ** (counter_bit_level2)
                        for index, item in enumerate(Level[2][bucket_index1]):
                            if item != 0:
                                if item[0] == FP_level2 and item[1] == 2 ** counter_bit_level2 - 1:
                                    Level[2][bucket_index1][index] = 0

def Clear_l2(lst,clear_num):
    for times in range(clear_num):
        Delete_Min_l2(lst)

def Delete_Min_l2(lst):
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

def Clear_l1(lst8_7, clear_num):
    if clear_num <= 0:
        return
    for times in range(clear_num):
        Delete_Min_l1(lst8_7)

def Delete_Min_l1(lst8_7):
    flag = 0
    min_num = sys.maxsize
    min_pos_8_7 = -1

    for i in range(len(lst8_7)):
        if lst8_7[i] != 0:
            if lst8_7[i][1]*2 < min_num:
                min_num = lst8_7[i][1]*2
                min_pos_8_7 = i
                flag = 7
        else:
            lst8_7.pop(i)
            return

    if flag == 7:
        lst8_7.pop(min_pos_8_7)
    return

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
                break
        
        print(f"Successfully read in {file_path}, {count} items")
    
    return keys

if __name__ == '__main__':
    # read file
    # df = utils.File_Util.read_single_file(49, True, unique_tag=False)
    # F = df['tag1']
    F = read_data('../../C++/data/20190117-130000-new.dat')
    print('The numbers of packets:{}'.format(len(F)))
    l = collections.Counter(F)

    for _T in [750, 1500, 3000, 6000]:
        top = _T * 4 / 3
        lst_PR = []
        lst_RR = []
        temp_memo = [50]
        if _T == 750:
            temp_memo = [50, 100, 150, 200, 250, 300]
        for _memorysize in temp_memo:
            memorysize = _memorysize
            alpha = 0.5
            bottom = top * alpha
            T = int((top + bottom) / 2)
            topK = 1024
            count_levelup = [0,0,0]
            b = 1.08
            k = 0.6

            counter_bit_level1 = 7
            fp_bit_level1 = 8

            counter_bit_level2 = 12
            fp_bit_level2 = 12

            counter_bit_level3 = 15
            fp_bit_level3 = 16

            #length of the two-mode active counter's exponent part
            gamma = 4

            #bits of one bucket
            W = 128

            meta = 4

            init_l1_num = 8

            BUCKET_NUM = int(memorysize*1024*8/W)

            print('Memory Size:{}KB'.format(memorysize))
            print('W:{}bits'.format(W))
            print('T:{}'.format(T))
            print('alpha:{}'.format(alpha))
            print('bottom:{},top:{}'.format(bottom, top))
            # print('The number of buckets:{}'.format(BUCKET_NUM))
            # print('The number of initial level-1 cell in each bucket:{}'.format(cell_num))

            #15 theta,v = 8,7
            Level1_1 = [[0 for j in range(init_l1_num)] for i in range(BUCKET_NUM)]
            #24 theta,v = 12,12
            Level2 = [[] for i in range(BUCKET_NUM)]
            #31 theta,v = 16,15
            Level3 = [[] for i in range(BUCKET_NUM)]

            Level = [[],Level1_1,Level2,Level3]
            #[C3,C2] in each bucket
            lst_index = [[0,0] for i in range(BUCKET_NUM)]
            # auxiliary set
            set_HH = set()

            SEED = [2132,315,1651,3165,4651]
            # SEED = [0, 1, 2, 3, 4, 5]
            for item in F:
                bucket_index1 = mmh3.hash(item, SEED[0]) % BUCKET_NUM

                FP_level1 = mmh3.hash(item,SEED[1]) % (2 ** fp_bit_level1)
                FP_level2 = mmh3.hash(item,SEED[3]) % (2 ** fp_bit_level2)
                FP_level3 = mmh3.hash(item,SEED[4]) % (2 ** fp_bit_level3)
                query_result = Query(item, bucket_index1, FP_level1, FP_level2, FP_level3)

                #query_result = [index,x] type-x index-th
                if query_result:
                    if query_result[1] >= 2:
                        if isinstance(Level[query_result[1]][bucket_index1][query_result[0]][1],int):
                            if Level[query_result[1]][bucket_index1][query_result[0]][1] == T:
                                set_HH.add(item)

                if query_result != 0:
                    query_index = query_result[0]
                    query_level = query_result[1]
                    if query_level == 3:
                        # until 1000000000000000 = 2 ** (counter_bit_level3 - 1)
                        if isinstance(Level[3][bucket_index1][query_index][1],int):

                            if Level[3][bucket_index1][query_index][1] < 2 ** (counter_bit_level3 - 1) - 1:
                                Level[3][bucket_index1][query_index][1] += 1

                            elif Level[3][bucket_index1][query_index][1] == 2 ** (counter_bit_level3 - 1) - 1:
                                # [exponent part, coefficient part]
                                Level[3][bucket_index1][query_index][1] = [0, 2 ** (counter_bit_level3 - 1 - gamma)]
                        else:
                            exponent_part = Level[3][bucket_index1][query_index][1][0]
                            coefficient_part = Level[3][bucket_index1][query_index][1][1]
                            if random.random() < 1/(2**(exponent_part + gamma)):
                                Level[3][bucket_index1][query_index][1][1] += 1
                                if Level[3][bucket_index1][query_index][1][1] >= 2 ** (counter_bit_level3 - gamma) - 1:
                                    Level[3][bucket_index1][query_index][1][0] += 1
                                    Level[3][bucket_index1][query_index][1][1] = 2 ** (counter_bit_level3 - 1 - gamma)

                    elif query_level == 2:
                        if Level[2][bucket_index1][query_index][1] < 2**counter_bit_level2-1:
                            Level[2][bucket_index1][query_index][1] += 1
                        else:
                            query_FP = Level[2][bucket_index1][query_index][0]
                            Switch(2, lst_index[bucket_index1][0], lst_index[bucket_index1][1])

                    elif query_level == 1:
                        if Level[1][bucket_index1][query_index][1] < 2**(counter_bit_level1)-1:
                            if random.random() < 1/2:
                                Level[1][bucket_index1][query_index][1] += 1
                        else:
                            Switch(1, lst_index[bucket_index1][0], lst_index[bucket_index1][1])

                #not recorded
                else:
                    if len(Level[1][bucket_index1]) > 0:
                        noEmpty = True
                        for index, item in enumerate(Level[1][bucket_index1]):
                            if item == 0:
                                if random.random() < 1 / 2:
                                    Level[1][bucket_index1][index] = [FP_level1, 1]
                                noEmpty = False
                                break

                        if noEmpty:
                            flag = 0
                            min_num_level1 = sys.maxsize
                            min_pos_level1_87 = -1
                            for i in range(len(Level[1][bucket_index1])):
                                if Level[1][bucket_index1] != 0:
                                    if Level[1][bucket_index1][i][1] * 2 < min_num_level1:
                                        min_num_level1 = Level[1][bucket_index1][i][1] * 2
                                        min_pos_level1_87 = i
                                        flag = 7

                            if min_num_level1 == sys.maxsize:
                                gailv_minus = 0
                            else:
                                gailv_minus = 1 / (b ** (math.log2(min_num_level1)))


                            if random.random() < gailv_minus/2:
                                Level[1][bucket_index1][min_pos_level1_87][1] -= 1

                                if Level[1][bucket_index1][min_pos_level1_87][1] <= 0:
                                    Level[1][bucket_index1][min_pos_level1_87][0] = FP_level1
                                    Level[1][bucket_index1][min_pos_level1_87][1] = 1


                    elif len(Level[2][bucket_index1]) > 0:
                        noEmpty = True
                        for index, item in enumerate(Level[2][bucket_index1]):
                            if item == 0:
                                Level[2][bucket_index1][index] = [FP_level2, 1]
                                noEmpty = False
                                break
                        if noEmpty:
                            min_num_level2 = sys.maxsize
                            min_pos_level2 = -1
                            for i in range(len(Level[2][bucket_index1])):
                                if Level[2][bucket_index1] != 0:
                                    if Level[2][bucket_index1][i][1] < min_num_level2:
                                        min_num_level2 = Level[2][bucket_index1][i][1]
                                        min_pos_level2 = i
                            # print(min_num_level1)

                            if min_num_level2 == sys.maxsize:
                                gailv_minus = 0
                            else:
                                gailv_minus = 1 / (b ** (math.log2(min_num_level2)))

                            if min_num_level2 < 256:
                                if random.random() < gailv_minus:
                                    Level[2][bucket_index1][min_pos_level2][1] -= 1

                                    if Level[2][bucket_index1][min_pos_level2][1] <= 0:
                                        Level[2][bucket_index1][min_pos_level2][0] = FP_level2
                                        Level[2][bucket_index1][min_pos_level2][1] = 1
                            else:
                                if random.random() < gailv_minus:
                                    Level[2][bucket_index1][min_pos_level2][1] -= 1

                                    if Level[2][bucket_index1][min_pos_level2][1] <= 255:
                                        Level[2][bucket_index1][min_pos_level2][0] = FP_level2
                                        Level[2][bucket_index1][min_pos_level2][1] = 256

                    else:
                        noEmpty = True
                        for index, item in enumerate(Level[3][bucket_index1]):
                            if item == 0:
                                Level[3][bucket_index1][index] = [FP_level3, 1]
                                noEmpty = False
                                break
                        if noEmpty:
                            min_num_level3 = sys.maxsize
                            min_pos_level3 = -1
                            for i in range(len(Level[3][bucket_index1])):
                                if Level[3][bucket_index1] != 0:
                                    if isinstance(Level[3][bucket_index1][i][1], int):
                                        if Level[3][bucket_index1][i][1] < min_num_level3:
                                            min_num_level3 = Level[3][bucket_index1][i][1]
                                            min_pos_level3 = i
                                    else:
                                        v = Level[3][bucket_index1][i][1][1] * (2 ** (gamma + Level[3][bucket_index1][i][1][0]))
                                        if v < min_num_level3:
                                            min_num_level3 = v
                                            min_pos_level3 = i

                            if min_num_level3 == sys.maxsize:
                                gailv_minus = 0
                            else:
                                gailv_minus = 1 / (b ** (math.log2(min_num_level3)))

                            if min_num_level3 < 4096:
                                if random.random() < gailv_minus:
                                    Level[3][bucket_index1][min_pos_level3][1] -= 1

                                    if Level[3][bucket_index1][min_pos_level3][1] <= 0:
                                        Level[3][bucket_index1][min_pos_level3][0] = FP_level3
                                        Level[3][bucket_index1][min_pos_level3][1] = 1
                            else:
                                if random.random() < gailv_minus:
                                    if isinstance(Level[3][bucket_index1][min_pos_level3][1], int):
                                        Level[3][bucket_index1][min_pos_level3][1] -= 1

                                        if Level[3][bucket_index1][min_pos_level3][1] <= 4095:
                                            Level[3][bucket_index1][min_pos_level3][0] = FP_level3
                                            Level[3][bucket_index1][min_pos_level3][1] = 4096

            lst_tem = []
            for item in lst_index:
                if item not in lst_tem:
                    lst_tem.append(item)
            print(lst_tem)

            #test
            lst_tem2 = []
            lst_legal = [[8, 0, 0], [6, 1, 0], [5, 2, 0], [3, 3, 0], [1, 4, 0], [0, 5, 0], [6, 0, 1], [4, 1, 1], [3, 2, 1], [1, 3, 1], [4, 0, 2], [2, 1, 2], [0, 2, 2], [2, 0, 3], [0, 1, 3], [0, 0, 4]]
            for i in range(BUCKET_NUM):
                tem = [len(Level[1][i]),len(Level[2][i]),len(Level[3][i])]
                if tem not in lst_legal:
                    lst_tem2.append(tem)
            print(lst_tem2)

            l1 = l.most_common(topK)
            count = 0
            #estimated
            result_overT = {}
            result_all = {}
            #real
            real = {}

            real_less_than_bottom = {}
            real_larger_than_upper = {}

            for i in l.items():
                if i[1] > T:
                    real.update({i[0]:i[1]})
                if i[1] < bottom:
                    real_less_than_bottom.update({i[0]:i[1]})
                if i[1] > top:
                    real_larger_than_upper.update({i[0]:i[1]})

            for key in set_HH:
                ID1 = mmh3.hash(key, SEED[0]) % BUCKET_NUM

                FP_l1 = mmh3.hash(key, SEED[1]) % (2 ** fp_bit_level1)
                FP_l2 = mmh3.hash(key, SEED[3]) % (2 ** fp_bit_level2)
                FP_l3 = mmh3.hash(key, SEED[4]) % (2 ** fp_bit_level3)
                query_result1 = Query(key, ID1,FP_l1,FP_l2,FP_l3)

                value_result = 0

                if query_result1 != 0:
                    query_index = query_result1[0]
                    query_level = query_result1[1]
                    if query_level == 3:
                        if not isinstance(Level[3][ID1][query_index][1],int):
                            value_result += Level[3][ID1][query_index][1][1]*(
                                    2**(Level[3][ID1][query_index][1][0] + gamma))
                        else:
                            value_result += Level[3][ID1][query_index][1]
                    elif query_level == 2:
                        value_result += Level[2][ID1][query_index][1]
                    elif query_level == 1:
                        value_result += 2 * Level[1][ID1][query_index][1]
                    else:
                        continue

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
                    del  real_larger_than_upper[key]

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
                aae_topk += abs(l[item[0]]-item[1])
                are_topk += abs(l[item[0]]-item[1])/l[item[0]]
                if item[0] in dic_l1:
                    count_topk += 1
                    del dic_l1[item[0]]

            FPR = count_bottom/len(real_less_than_bottom)
            FNR = count_upper_mis/count_upper_all
            print('Top-theta aae:{},Top-theta are:{}'.format(aae_topk/topK,are_topk/topK))
            print('OverT aae:{},OverT are:{}'.format(aae_overT/len(result_overT),are_overT/len(result_overT)))
            print(count_bottom,len(real_less_than_bottom),count_upper_mis,count_upper_all)
            print('FPR:{},FNR:{}'.format(FPR,FNR))
            print(count, len(real), len(result_overT))
            print('Find top-{}:{}'.format(topK, count_topk))
            print('Threshold={},PR:{}'.format(T, count / len(result_overT)))
            print('Threshold={},RR:{}'.format(T, count / len(real)))
            lst_PR.append(count / len(result_overT))
            lst_RR.append(count / len(real))
            print('PR',lst_PR)
            print('RR',lst_RR)

            PR = count / len(result_overT)
            RR = count / len(real)

            if _T == 750 and _memorysize == 150:
                with open('./result/FNR.txt', 'a') as f:
                    f.write(f"SwitchSketch\n{FNR}\n")
                with open('./result/FPR.txt', 'a') as f:
                    f.write(f"SwitchSketch\n{FPR}\n")

            if _T == 750:
                with open('./result/F2_memory.txt', 'a') as f:
                    if _memorysize == 50:
                        f.write(f"SwitchSketch\n{5*PR*RR/(4*PR+RR)}")
                    else:
                        f.write(f" {5*PR*RR/(4*PR+RR)}")
                    if _memorysize == 300:
                        f.write('\n')

            if _memorysize == 50:
                with open('./result/F2_t.txt', 'a') as f:
                    if _T == 750:
                        f.write(f"SwitchSketch\n{5*PR*RR/(4*PR+RR)}")
                    else:
                        f.write(f" {5*PR*RR/(4*PR+RR)}")
                    if _T == 6000:
                        f.write('\n')