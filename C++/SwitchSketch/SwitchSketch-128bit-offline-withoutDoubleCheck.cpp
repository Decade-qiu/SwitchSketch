#include <vector>
#include <string.h>
#include <iostream>
#include <cmath>
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <cstdint>

using namespace std;
#define b 1.08          //exponential decay parameter
#define BUCKET_NUM 6400 //the number of bucket 2*6400*128bits=200KB
#define KEY_SIZE 4      //size of key

#define THRESHOLD 750

//frequently used value
#define FP_LEN1 8
#define FP_LEN2 12
#define FP_LEN3 16

#define FP_MASK1 0xFF
#define FP_MASK2 0xFFF
#define FP_MASK3 0xFFFF

#define CT_MASK1 0x7F
#define CT_MASK2 0xFFF
#define CT_MASK3 0x7FFF

#define CELL_LEN1 15
#define CELL_LEN2 24
#define CELL_LEN3 31

#define CELL_MASK1 0x7FFF
#define CELL_MASK2 0xFFFFFF
#define CELL_MASK3 0x7FFFFFFF

#define META_LENGTH 4
#define META_MASK 0xF
#define BUCKET_CELLS_LEGTH 124

#define EXP_MODE_MASK 0x4000
//counters >= this threshold cannot be directly decayed
#define EXP_MODE_T 0x4001
//mask for reset the counting part of the two mode active counter
#define EXP_CT_MASK 0x3FF0
//mask to check if the counting part will overflow
#define EXP_CT_OVFL_MASK 0x7FF0

#define MAX_NUM_LV1 8
#define MAX_NUM_LV2 5
#define MAX_NUM_LV3 4

//value of counters that just Switch (not the represented value of counters, just the value directly read from counter)
#define MIN_C_LV_2 256
#define MIN_C_LV_3 4096


uint64_t B[BUCKET_NUM][2]{0};//array of buckets
uint8_t **keys = (uint8_t **) malloc(sizeof(uint8_t * ) * 40000000); //array of keys

inline uint32_t BKDRHash(const uint8_t *str, uint32_t len) {
    uint32_t seed = 13131;
    uint32_t hash = 0;

    for (uint32_t i = 0; i < len; i++) {
        hash = hash * seed + str[i];
    }

    return (hash & 0x7FFFFFFF);
}

inline uint16_t finger_print(uint32_t hash) {
    hash ^= hash >> 16;
    hash *= 0x85ebca6b;
    hash ^= hash >> 13;
    hash *= 0xc2b2ae35;
    hash ^= hash >> 16;
    return hash & 65535;
}

bool cmpPairFunc(pair<uint8_t *, int> p1, pair<uint8_t *, int> p2) {
    return p1.second > p2.second;
}

struct CmpFunc {
    bool operator()(const uint8_t *keyA, const uint8_t *keyB) const {
        return memcmp(keyA, keyB, KEY_SIZE) == 0;
    }
};

struct HashFunc {
    uint32_t operator()(const uint8_t *key) const {
        uint32_t hashValue = BKDRHash(key, KEY_SIZE);
        return hashValue;
    }
};

inline void metaCodeToData(uint8_t metaCode, int &num_lv_1, int &num_lv_2, int &num_lv_3) {
    switch (metaCode) {
        case (0):
            num_lv_3 = 0;
            num_lv_2 = 0;
            num_lv_1 = 8;
            break;
        case (1):
            num_lv_3 = 0;
            num_lv_2 = 1;
            num_lv_1 = 6;
            break;
        case (2):
            num_lv_3 = 0;
            num_lv_2 = 2;
            num_lv_1 = 5;
            break;
        case (3):
            num_lv_3 = 0;
            num_lv_2 = 3;
            num_lv_1 = 3;
            break;
        case (4):
            num_lv_3 = 0;
            num_lv_2 = 4;
            num_lv_1 = 1;
            break;
        case (5):
            num_lv_3 = 0;
            num_lv_2 = 5;
            num_lv_1 = 0;
            break;
        case (6):
            num_lv_3 = 1;
            num_lv_2 = 0;
            num_lv_1 = 6;
            break;
        case (7):
            num_lv_3 = 1;
            num_lv_2 = 1;
            num_lv_1 = 4;
            break;
        case (8):
            num_lv_3 = 1;
            num_lv_2 = 2;
            num_lv_1 = 3;
            break;
        case (9):
            num_lv_3 = 1;
            num_lv_2 = 3;
            num_lv_1 = 1;
            break;
        case (10):
            num_lv_3 = 2;
            num_lv_2 = 0;
            num_lv_1 = 4;
            break;
        case (11):
            num_lv_3 = 2;
            num_lv_2 = 1;
            num_lv_1 = 2;
            break;
        case (12):
            num_lv_3 = 2;
            num_lv_2 = 2;
            num_lv_1 = 0;
            break;
        case (13):
            num_lv_3 = 3;
            num_lv_2 = 0;
            num_lv_1 = 2;
            break;
        case (14):
            num_lv_3 = 3;
            num_lv_2 = 1;
            num_lv_1 = 0;
            break;
        case (15):
            num_lv_3 = 4;
            num_lv_2 = 0;
            num_lv_1 = 0;
            break;
        default:
            break;
    }
}

int getMetaCode(int new_num_lv_2, int new_num_lv_3) {
    switch (new_num_lv_3) {
        case 0:
            switch (new_num_lv_2) {
                case 0:
                    return 0;
                case 1:
                    return 1;
                case 2:
                    return 2;
                case 3:
                    return 3;
                case 4:
                    return 4;
                case 5:
                    return 5;
                default:
                    return -1;
            }
            break;
        case 1:
            switch (new_num_lv_2) {
                case 0:
                    return 6;
                case 1:
                    return 7;
                case 2:
                    return 8;
                case 3:
                    return 9;
                default:
                    return -1;
            }
            break;
        case 2:
            switch (new_num_lv_2) {
                case 0:
                    return 10;
                case 1:
                    return 11;
                case 2:
                    return 12;
                default:
                    return -1;
            }
            break;
        case 3:
            switch (new_num_lv_2) {
                case 0:
                    return 13;
                case 1:
                    return 14;
                default:
                    return -1;
            }
            break;
        case 4:
            if (new_num_lv_2 == 0) {
                return 15;
            } else {
                cout << "encode error!";
                return -1;
            }
    }
    return -1;
}

void findMinCell(uint64_t *bucket, int level, const int &num_lv_1, const int &num_lv_2, const int &num_lv_3,
                 uint16_t &min_counter, uint16_t &min_index) {
    switch (level) {
        case 1: {
            min_counter = -1;
            min_index = -1;
            uint32_t start_lv1 = META_LENGTH + num_lv_3 * CELL_LEN3 + num_lv_2 * CELL_LEN2;
            uint32_t end_lv1 = start_lv1 + num_lv_1 * CELL_LEN1;

            uint16_t tmp_counter = 0;
            for (uint32_t j = start_lv1 + FP_LEN1; j < end_lv1; j += CELL_LEN1) {
                tmp_counter = ((*(uint32_t * )((uint8_t *) bucket + (j >> 3))) >> (j & 0x7)) & CT_MASK1;

                if (tmp_counter < min_counter) {
                    min_counter = tmp_counter;
                    min_index = j - FP_LEN1;
                }
            }
            return;
        }
        case 2: {
            min_counter = -1;
            min_index = -1;
            uint32_t start_lv2 = META_LENGTH + num_lv_3 * CELL_LEN3;
            uint32_t end24 = start_lv2 + num_lv_2 * CELL_LEN2;

            uint16_t tmp_counter = 0;
            for (uint32_t j = start_lv2 + FP_LEN2; j < end24; j += CELL_LEN2) {
                tmp_counter = ((*(uint32_t * )((uint8_t *) bucket + (j >> 3))) >> (j & 0x7)) & CT_MASK2;

                if (tmp_counter < min_counter) {
                    min_counter = tmp_counter;
                    min_index = j - FP_LEN2;
                }
            }
            return;
        }
        case 3://only find the counters in normal mode
        {
            min_counter = EXP_MODE_T;
            min_index = -1;
            uint16_t tmp_counter = 0;
            uint32_t start_lv2 = META_LENGTH + num_lv_3 * CELL_LEN3;

            for (uint32_t j = META_LENGTH + FP_LEN3; j < start_lv2; j += CELL_LEN3) {
                tmp_counter = ((*(uint32_t * )((uint8_t *) bucket + (j >> 3))) >> (j & 0x7)) & CT_MASK3;

                if (tmp_counter < min_counter) {
                    min_counter = tmp_counter;
                    min_index = j - FP_LEN3;
                }
            }
            return;
        }
    }
}

void Switch(uint64_t *bucket, int level, const int &num_lv_1, const int &num_lv_2, const int &num_lv_3, uint16_t fp16,
            uint32_t cell_start_bit_idx) {
    int new_num_lv_1 = 0, new_num_lv_2 = 0, new_num_lv_3 = 0; //new number of level_1,level_2,level_3 cells
    int start_lv2 = 0, start_lv1 = 0, end_lv1 = 0;            //start index in bit
    int new_start_lv2 = 0, new_start_lv1 = 0;      //new start index in bit


    uint8_t cell_lv_1[MAX_NUM_LV1][2] = {0}; //Temporary variable to store flow in the bucket
    uint16_t cell_lv_2[MAX_NUM_LV2][2] = {0};
    uint16_t cell_lv_3[MAX_NUM_LV3][2] = {0};

    int usage_lv_1 = 0, usage_lv_2 = 0, usage_lv_3 = 0;        //number of used level_1,level_2,level_3 cells
    int i = 0, j = 0;
    double ranf = 0; //random number for exponential decay
    uint16_t tmp_fp = 0, tmp_counter = 0;

    uint16_t min_counter = 0;
    uint16_t min_index = -1;

    switch (level) {
        case 2: {
            new_num_lv_2 = num_lv_2 + 1; //get new_num_lv_2
            new_num_lv_3 = num_lv_3;     //get new_num_lv_3
            int bits_remain = BUCKET_CELLS_LEGTH - CELL_LEN3 * new_num_lv_3 - CELL_LEN2 * new_num_lv_2;

            //levelup success
            if (bits_remain >= 0) {
                new_num_lv_1 = bits_remain / CELL_LEN1;

                start_lv2 = META_LENGTH + num_lv_3 * CELL_LEN3;
                start_lv1 = start_lv2 + num_lv_2 * CELL_LEN2;
                end_lv1 = start_lv1 + num_lv_1 * CELL_LEN1;

                //Temporary store level_3 flows
                for (usage_lv_3 = 0, j = META_LENGTH; j < start_lv2; j += CELL_LEN3) {
                    tmp_fp = ((*(uint32_t * )((uint8_t *) bucket + (j >> 3))) >> (j & 0x7)) & FP_MASK3;
                    tmp_counter =
                            ((*(uint32_t * )((uint8_t *) bucket + ((j + FP_LEN3) >> 3))) >> ((j + FP_LEN3) & 0x7)) &
                            CT_MASK3;

                    if (tmp_counter > 0) {
                        cell_lv_3[usage_lv_3][0] = tmp_fp;
                        cell_lv_3[usage_lv_3][1] = tmp_counter;
                        usage_lv_3++;
                    }
                }
                //Temporary store level_2 flows
                for (usage_lv_2 = 0, j = start_lv2; j < start_lv1; j += CELL_LEN2) {
                    tmp_fp = ((*(uint32_t * )((uint8_t *) bucket + (j >> 3))) >> (j & 0x7)) & FP_MASK2;
                    tmp_counter =
                            ((*(uint32_t * )((uint8_t *) bucket + ((j + FP_LEN2) >> 3))) >> ((j + FP_LEN2) & 0x7)) &
                            CT_MASK2;

                    if (tmp_counter > 0) {
                        cell_lv_2[usage_lv_2][0] = tmp_fp;
                        cell_lv_2[usage_lv_2][1] = tmp_counter;
                        usage_lv_2++;
                    }
                }
                //Temporary store  fp16
                cell_lv_2[usage_lv_2][0] = fp16 & FP_MASK2;
                cell_lv_2[usage_lv_2][1] = MIN_C_LV_2;
                usage_lv_2++;

                uint16_t min_f = 0; //Minimum flow
                //Temporary store level_1 flows except fp16 and min_f
                for (j = start_lv1; j < end_lv1; j += CELL_LEN1) {
                    tmp_fp = ((*(uint32_t * )((uint8_t *) bucket + (j >> 3))) >> (j & 0x7)) & FP_MASK1;
                    tmp_counter =
                            ((*(uint32_t * )((uint8_t *) bucket + ((j + FP_LEN1) >> 3))) >> ((j + FP_LEN1) & 0x7)) &
                            CT_MASK1;

                    if (tmp_counter && (tmp_fp != (fp16 & FP_MASK1))) {
                        if (!min_counter) {
                            min_f = tmp_fp;
                            min_counter = tmp_counter;
                        } else if (tmp_counter < min_counter) {
                            cell_lv_1[usage_lv_1][0] = min_f;
                            cell_lv_1[usage_lv_1][1] = min_counter;
                            min_f = tmp_fp;
                            min_counter = tmp_counter;
                            usage_lv_1++;
                        } else {
                            cell_lv_1[usage_lv_1][0] = tmp_fp;
                            cell_lv_1[usage_lv_1][1] = tmp_counter;
                            usage_lv_1++;
                        }
                    }
                }

                //if new level_1 cell is not full, store min_f
                if (min_counter > 0 && usage_lv_1 < new_num_lv_1) {
                    cell_lv_1[usage_lv_1][0] = min_f;
                    cell_lv_1[usage_lv_1][1] = min_counter;
                    usage_lv_1++;
                }
                //Temporary store finished

                //flush bucket
                //change when bucket structure changes
                bucket[0] = 0;
                bucket[1] = 0;

                new_start_lv2 = META_LENGTH + new_num_lv_3 * CELL_LEN3;
                new_start_lv1 = new_start_lv2 + new_num_lv_2 * CELL_LEN2;


                //insert stored level_3 flows
                for (j = META_LENGTH, i = 0; i < usage_lv_3; i++, j += CELL_LEN3) {
                    *(uint32_t * )((uint8_t *) bucket + (j >> 3)) |= ((uint32_t) cell_lv_3[i][0]) << (j & 0x7);
                    *(uint32_t * )((uint8_t *) bucket + ((j + FP_LEN3) >> 3)) |=
                            ((uint32_t) cell_lv_3[i][1]) << ((j + FP_LEN3) & 0x7);
                }
                //insert stored level_2 flows
                for (j = new_start_lv2, i = 0; i < usage_lv_2; i++, j += CELL_LEN2) {
                    *(uint32_t * )((uint8_t *) bucket + (j >> 3)) |= ((uint32_t) cell_lv_2[i][0]) << (j & 0x7);
                    *(uint32_t * )((uint8_t *) bucket + ((j + FP_LEN2) >> 3)) |=
                            ((uint32_t) cell_lv_2[i][1]) << ((j + FP_LEN2) & 0x7);
                }
                //insert stored level_1 flows
                for (j = new_start_lv1, i = 0; i < usage_lv_1; i++, j += CELL_LEN1) {
                    *(uint32_t * )((uint8_t *) bucket + (j >> 3)) |= ((uint32_t) cell_lv_1[i][0]) << (j & 0x7);
                    *(uint32_t * )((uint8_t *) bucket + ((j + FP_LEN1) >> 3)) |=
                            ((uint32_t) cell_lv_1[i][1]) << ((j + FP_LEN1) & 0x7);
                }

                int metaCode = getMetaCode(new_num_lv_2, new_num_lv_3);
                bucket[0] |= metaCode & META_MASK;
                return;
            } else//levelup not success,exponential decay, change when bucket structure changes
            {
                findMinCell(bucket, 2, num_lv_1, num_lv_2, num_lv_3, min_counter, min_index);
                //exponential decay
                ranf = 1.0 * rand() / RAND_MAX;
                if (ranf < pow(b, log2(min_counter) * -1)) {
                    if (min_counter <= MIN_C_LV_2) {
                        *(uint32_t * )((uint8_t *) bucket + (cell_start_bit_idx >> 3)) &= ~(((uint32_t) CELL_MASK1)
                                << (cell_start_bit_idx & 0x7));//clear original cell

                        //replace fp
                        *(uint32_t * )((uint8_t *) bucket + (min_index >> 3)) &= ~(((uint32_t) FP_MASK2)
                                << (min_index & 0x7));
                        *(uint32_t * )((uint8_t *) bucket + (min_index >> 3)) |=
                                ((uint32_t) fp16 & FP_MASK2) << (min_index & 0x7);
                        *(uint32_t * )((uint8_t *) bucket + ((min_index + FP_LEN2) >> 3)) &= ~(((uint32_t) FP_MASK2)
                                << ((min_index + FP_LEN2) & 0x7));
                        *(uint32_t * )((uint8_t *) bucket + ((min_index + FP_LEN2) >> 3)) |=
                                ((uint32_t) MIN_C_LV_2) << ((min_index + FP_LEN2) & 0x7);
                        return;
                    } else//decay
                    {
                        *(uint32_t * )((uint8_t *) bucket + ((min_index + FP_LEN2) >> 3)) -=
                                ((uint32_t) 1) << ((min_index + FP_LEN2) & 0x7);
                        return;
                    }
                }
            }
            break;
        }
        case 3: {
            new_num_lv_2 = num_lv_2 - 1; //get new_num_lv_2
            new_num_lv_3 = num_lv_3 + 1;     //get new_num_lv_3
            int bits_remain = BUCKET_CELLS_LEGTH - CELL_LEN3 * new_num_lv_3 - CELL_LEN2 * new_num_lv_2;

            if (bits_remain >= 0) {//remove a CELL_LEN1-bit cell and the original CELL_LEN2-bit cell
                new_num_lv_1 = bits_remain / CELL_LEN1;

                start_lv2 = META_LENGTH + num_lv_3 * CELL_LEN3;
                start_lv1 = start_lv2 + num_lv_2 * CELL_LEN2;
                end_lv1 = start_lv1 + num_lv_1 * CELL_LEN1;

                for (usage_lv_3 = 0, j = META_LENGTH; j < start_lv2; j += CELL_LEN3) {

                    tmp_fp = ((*(uint32_t * )((uint8_t *) bucket + (j >> 3))) >> (j & 0x7)) & FP_MASK3;
                    tmp_counter =
                            ((*(uint32_t * )((uint8_t *) bucket + ((j + FP_LEN3) >> 3))) >> ((j + FP_LEN3) & 0x7)) &
                            CT_MASK3;

                    if (tmp_counter > 0) {
                        cell_lv_3[usage_lv_3][0] = tmp_fp;
                        cell_lv_3[usage_lv_3][1] = tmp_counter;
                        usage_lv_3++;
                    }
                }
                //store fp16
                cell_lv_3[usage_lv_3][0] = fp16;
                cell_lv_3[usage_lv_3][1] = MIN_C_LV_3;
                usage_lv_3++;

                //Temporary store level_2 flows except fp16
                for (usage_lv_2 = 0, j = start_lv2; j < start_lv1; j += CELL_LEN2) {
                    tmp_fp = ((*(uint32_t * )((uint8_t *) bucket + (j >> 3))) >> (j & 0x7)) & FP_MASK2;
                    tmp_counter =
                            ((*(uint32_t * )((uint8_t *) bucket + ((j + FP_LEN2) >> 3))) >> ((j + FP_LEN2) & 0x7)) &
                            CT_MASK2;

                    if (tmp_counter && (tmp_fp != (fp16 & FP_MASK2))) {
                        cell_lv_2[usage_lv_2][0] = tmp_fp;
                        cell_lv_2[usage_lv_2][1] = tmp_counter;
                        usage_lv_2++;
                    }
                }

                uint16_t min_f = 0; //Minimum flow
                //Temporary store level_1 flows except the min
                for (j = start_lv1; j < end_lv1; j += CELL_LEN1) {
                    tmp_fp = ((*(uint32_t * )((uint8_t *) bucket + (j >> 3))) >> (j & 0x7)) & FP_MASK1;
                    tmp_counter =
                            ((*(uint32_t * )((uint8_t *) bucket + ((j + FP_LEN1) >> 3))) >> ((j + FP_LEN1) & 0x7)) &
                            CT_MASK1;

                    if (tmp_counter) {
                        if (!min_counter) {
                            min_f = tmp_fp;
                            min_counter = tmp_counter;
                        } else if (tmp_counter < min_counter) {
                            cell_lv_1[usage_lv_1][0] = min_f;
                            cell_lv_1[usage_lv_1][1] = min_counter;
                            min_f = tmp_fp;
                            min_counter = tmp_counter;
                            usage_lv_1++;
                        } else {
                            cell_lv_1[usage_lv_1][0] = tmp_fp;
                            cell_lv_1[usage_lv_1][1] = tmp_counter;
                            usage_lv_1++;
                        }
                    }
                }
                //if new level_1 cell is not full, store min_f
                if (usage_lv_1 < new_num_lv_1) {
                    cell_lv_1[usage_lv_1][0] = min_f;
                    cell_lv_1[usage_lv_1][1] = min_counter;
                    usage_lv_1++;
                }
                //Temporary store finished

                //flush bucket
                //change when bucket structure changes
                bucket[0] = 0;
                bucket[1] = 0;

                new_start_lv2 = META_LENGTH + new_num_lv_3 * CELL_LEN3;
                new_start_lv1 = new_start_lv2 + new_num_lv_2 * CELL_LEN2;


                //insert stored level_3 flows
                for (j = META_LENGTH, i = 0; i < usage_lv_3; i++, j += CELL_LEN3) {
                    *(uint32_t * )((uint8_t *) bucket + (j >> 3)) |= ((uint32_t) cell_lv_3[i][0]) << (j & 0x7);
                    *(uint32_t * )((uint8_t *) bucket + ((j + FP_LEN3) >> 3)) |=
                            ((uint32_t) cell_lv_3[i][1]) << ((j + FP_LEN3) & 0x7);
                }
                //insert stored level_2 flows
                for (j = new_start_lv2, i = 0; i < usage_lv_2; i++, j += CELL_LEN2) {
                    *(uint32_t * )((uint8_t *) bucket + (j >> 3)) |= ((uint32_t) cell_lv_2[i][0]) << (j & 0x7);
                    *(uint32_t * )((uint8_t *) bucket + ((j + FP_LEN2) >> 3)) |=
                            ((uint32_t) cell_lv_2[i][1]) << ((j + FP_LEN2) & 0x7);
                }
                //insert stored level_1 flows
                for (j = new_start_lv1, i = 0; i < usage_lv_1; i++, j += CELL_LEN1) {
                    *(uint32_t * )((uint8_t *) bucket + (j >> 3)) |= ((uint32_t) cell_lv_1[i][0]) << (j & 0x7);
                    *(uint32_t * )((uint8_t *) bucket + ((j + FP_LEN1) >> 3)) |=
                            ((uint32_t) cell_lv_1[i][1]) << ((j + FP_LEN1) & 0x7);
                }
                int metaCode = getMetaCode(new_num_lv_2, new_num_lv_3);
                bucket[0] |= metaCode & META_MASK;
                return;
            }

            new_num_lv_2 = num_lv_2 - 2; //get new_num_lv_2
            new_num_lv_3 = num_lv_3 + 1;     //get new_num_lv_3
            bits_remain = BUCKET_CELLS_LEGTH - CELL_LEN3 * new_num_lv_3 - CELL_LEN2 * new_num_lv_2;

            if (bits_remain >= 0) {//remove another CELL_LEN2-bit cell and the original CELL_LEN2-bit cell
                new_num_lv_1 = bits_remain / CELL_LEN1;

                start_lv2 = META_LENGTH + num_lv_3 * CELL_LEN3;
                start_lv1 = start_lv2 + num_lv_2 * CELL_LEN2;
                end_lv1 = start_lv1 + num_lv_1 * CELL_LEN1;

                //Temporary store level_3 flows
                for (usage_lv_3 = 0, j = META_LENGTH; j < start_lv2; j += CELL_LEN3) {
                    tmp_fp = ((*(uint32_t * )((uint8_t *) bucket + (j >> 3))) >> (j & 0x7)) & FP_MASK3;
                    tmp_counter =
                            ((*(uint32_t * )((uint8_t *) bucket + ((j + FP_LEN3) >> 3))) >> ((j + FP_LEN3) & 0x7)) &
                            CT_MASK3;

                    if (tmp_counter > 0) {
                        cell_lv_3[usage_lv_3][0] = tmp_fp;
                        cell_lv_3[usage_lv_3][1] = tmp_counter;
                        usage_lv_3++;
                    }
                }
                //store fp16
                cell_lv_3[usage_lv_3][0] = fp16;
                cell_lv_3[usage_lv_3][1] = MIN_C_LV_3;
                usage_lv_3++;

                uint16_t min_f = 0; //Minimum flow
                //Temporary store level_2 flows except fp16 and min
                for (usage_lv_2 = 0, j = start_lv2; j < start_lv1; j += CELL_LEN2) {
                    tmp_fp = ((*(uint32_t * )((uint8_t *) bucket + (j >> 3))) >> (j & 0x7)) & FP_MASK2;
                    tmp_counter =
                            ((*(uint32_t * )((uint8_t *) bucket + ((j + FP_LEN2) >> 3))) >> ((j + FP_LEN2) & 0x7)) &
                            CT_MASK2;

                    if (tmp_counter && (tmp_fp != (fp16 & FP_MASK2))) {

                        if (!min_counter) {
                            min_f = tmp_fp;
                            min_counter = tmp_counter;
                        } else if (tmp_counter < min_counter) {
                            cell_lv_2[usage_lv_2][0] = min_f;
                            cell_lv_2[usage_lv_2][1] = min_counter;
                            min_f = tmp_fp;
                            min_counter = tmp_counter;
                            usage_lv_2++;
                        } else {
                            cell_lv_2[usage_lv_2][0] = tmp_fp;
                            cell_lv_2[usage_lv_2][1] = tmp_counter;
                            usage_lv_2++;
                        }
                    }
                }

                //if new level_2 cell is not full, store min_f
                if (min_counter > 0 && usage_lv_2 < new_num_lv_2) {
                    cell_lv_2[usage_lv_1][0] = min_f;
                    cell_lv_2[usage_lv_1][1] = min_counter;
                    usage_lv_2++;
                }

                //Temporary store level_1 flows except the min
                for (j = start_lv1; j < end_lv1; j += CELL_LEN1) {
                    tmp_fp = ((*(uint32_t * )((uint8_t *) bucket + (j >> 3))) >> (j & 0x7)) & FP_MASK1;
                    tmp_counter =
                            ((*(uint32_t * )((uint8_t *) bucket + ((j + FP_LEN1) >> 3))) >> ((j + FP_LEN1) & 0x7)) &
                            CT_MASK1;

                    if (tmp_counter) {
                        cell_lv_1[usage_lv_1][0] = tmp_fp;
                        cell_lv_1[usage_lv_1][1] = tmp_counter;
                        usage_lv_1++;
                    }
                }
                //Temporary store finished

                //flush bucket
                //change when bucket structure changes
                bucket[0] = 0;
                bucket[1] = 0;

                new_start_lv2 = META_LENGTH + new_num_lv_3 * CELL_LEN3;
                new_start_lv1 = new_start_lv2 + new_num_lv_2 * CELL_LEN2;


                //insert stored level_3 flows
                for (j = META_LENGTH, i = 0; i < usage_lv_3; i++, j += CELL_LEN3) {
                    *(uint32_t * )((uint8_t *) bucket + (j >> 3)) |= ((uint32_t) cell_lv_3[i][0]) << (j & 0x7);
                    *(uint32_t * )((uint8_t *) bucket + ((j + FP_LEN3) >> 3)) |=
                            ((uint32_t) cell_lv_3[i][1]) << ((j + FP_LEN3) & 0x7);
                }
                //insert stored level_2 flows
                for (j = new_start_lv2, i = 0; i < usage_lv_2; i++, j += CELL_LEN2) {
                    *(uint32_t * )((uint8_t *) bucket + (j >> 3)) |= ((uint32_t) cell_lv_2[i][0]) << (j & 0x7);
                    *(uint32_t * )((uint8_t *) bucket + ((j + FP_LEN2) >> 3)) |=
                            ((uint32_t) cell_lv_2[i][1]) << ((j + FP_LEN2) & 0x7);
                }
                //insert stored level_1 flows
                for (j = new_start_lv1, i = 0; i < usage_lv_1; i++, j += CELL_LEN1) {
                    *(uint32_t * )((uint8_t *) bucket + (j >> 3)) |= ((uint32_t) cell_lv_1[i][0]) << (j & 0x7);
                    *(uint32_t * )((uint8_t *) bucket + ((j + FP_LEN1) >> 3)) |=
                            ((uint32_t) cell_lv_1[i][1]) << ((j + FP_LEN1) & 0x7);
                }
                int metaCode = getMetaCode(new_num_lv_2, new_num_lv_3);
                bucket[0] |= metaCode & META_MASK;

                return;
            } else//levelup not success,exponential decay, change when bucket structure changes
            {
                findMinCell(bucket, 3, num_lv_1, num_lv_2, num_lv_3, min_counter, min_index);
                if (min_counter > EXP_MODE_MASK) {
                    return;
                }

                //exponential decay
                ranf = 1.0 * rand() / RAND_MAX;
                if (ranf < pow(b, log2(min_counter) * -1)) {
                    if (min_counter == MIN_C_LV_3)//only replace fingerprints
                    {
                        *(uint32_t * )((uint8_t *) bucket + (cell_start_bit_idx >> 3)) &= ~(((uint32_t) CELL_MASK2)
                                << (cell_start_bit_idx & 0x7));//clear original cell

                        //replace fp
                        *(uint32_t * )((uint8_t *) bucket + (min_index >> 3)) &= ~(((uint32_t) FP_MASK3)
                                << (min_index & 0x7));
                        *(uint32_t * )((uint8_t *) bucket + (min_index >> 3)) |=
                                ((uint32_t) fp16 & FP_MASK3) << (min_index & 0x7);
                        return;
                    } else//decay
                    {
                        *(uint32_t * )((uint8_t *) bucket + ((min_index + FP_LEN3) >> 3)) -=
                                ((uint32_t) 1) << ((min_index + FP_LEN3) & 0x7);
                        return;
                    }
                }
            }
        }
    }
}

void insert(int packet_cnt) {
    uint32_t hash;
    uint16_t bucket_index, fp16;
    uint16_t tmp_fp, tmp_counter;

    uint8_t meta;
    int num_lv_1, num_lv_2, num_lv_3;
    int start_lv2, start_lv1, end_lv1;

    int i, j;
    uint32_t ran;
    double ranf;

    bool half_flag = false;

    for (i = 0; i < packet_cnt; i++) {
        hash = BKDRHash(keys[i], KEY_SIZE);
        bucket_index = hash % BUCKET_NUM;         //get bucket index
        fp16 = finger_print(hash);                //get fp
        uint64_t *bucket = B[bucket_index];

        meta = bucket[0] & META_MASK;
        metaCodeToData(meta, num_lv_1, num_lv_2, num_lv_3);

        //each 256-bit bucket is composed of 4 64-bit words, all cells are stored from the lower addresses. In each cell, lower bits are the fingerprint, and higher bits are the counter.
        start_lv2 = META_LENGTH + num_lv_3 * CELL_LEN3;
        start_lv1 = start_lv2 + num_lv_2 * CELL_LEN2;
        end_lv1 = start_lv1 + num_lv_1 * CELL_LEN1;

        //if exists a flow in level_3
        for (j = META_LENGTH; j < start_lv2; j += CELL_LEN3) {
            tmp_fp = ((*(uint32_t * )((uint8_t *) bucket + (j >> 3))) >> (j & 0x7)) & FP_MASK3;
            tmp_counter =
                    ((*(uint32_t * )((uint8_t *) bucket + ((j + FP_LEN3) >> 3))) >> ((j + FP_LEN3) & 0x7)) & CT_MASK3;
            if (tmp_fp == fp16 && tmp_counter > 0) {
                if (!(tmp_counter & EXP_MODE_MASK))//normal mode
                {
                    *(uint32_t * )((uint8_t *) bucket + ((j + FP_LEN3) >> 3)) +=
                            ((uint32_t) 1) << ((j + FP_LEN3) & 0x7);
                } else//exponential mode
                {
                    ran = rand() & 0x7fff;
                    if (ran <= (1 << (15 - 4 - (tmp_counter & 0xF)))) {
                        if (tmp_counter < EXP_CT_OVFL_MASK) {
                            *(uint32_t * )((uint8_t *) bucket + ((j + FP_LEN3) >> 3)) +=
                                    ((uint32_t) 0x10) << ((j + FP_LEN3) & 0x7);
                        } else//coefficient part is overflow
                        {
                            *(uint32_t * )((uint8_t *) bucket + ((j + FP_LEN3) >> 3)) &= ~(((uint32_t) EXP_CT_MASK)
                                    << ((j + FP_LEN3) & 0x7));
                            *(uint32_t * )((uint8_t *) bucket + ((j + FP_LEN3) >> 3)) +=
                                    ((uint32_t) 1) << ((j + FP_LEN3) & 0x7);
                        }
                    }
                }
                goto pkt_done;
            }
        }

        //if existing flow in level_2
        for (j = start_lv2; j < start_lv1; j += CELL_LEN2) {
            tmp_fp = ((*(uint32_t * )((uint8_t *) bucket + (j >> 3))) >> (j & 0x7)) & FP_MASK2;
            tmp_counter =
                    ((*(uint32_t * )((uint8_t *) bucket + ((j + FP_LEN2) >> 3))) >> ((j + FP_LEN2) & 0x7)) & CT_MASK2;

            if (tmp_fp == (fp16 & FP_MASK2) && tmp_counter > 0) {
                if (tmp_counter != CT_MASK2) {
                    *(uint32_t * )((uint8_t *) bucket + ((j + FP_LEN2) >> 3)) +=
                            ((uint32_t) 1) << ((j + FP_LEN2) & 0x7);
                } else {
                    Switch(bucket, 3, num_lv_1, num_lv_2, num_lv_3, fp16, j);
                }
                goto pkt_done;
            }
        }
        //if existing flow in level_1
        for (j = start_lv1; j < end_lv1; j += CELL_LEN1) {
            tmp_fp = ((*(uint32_t * )((uint8_t *) bucket + (j >> 3))) >> (j & 0x7)) & FP_MASK1;
            tmp_counter =
                    ((*(uint32_t * )((uint8_t *) bucket + ((j + FP_LEN1) >> 3))) >> ((j + FP_LEN1) & 0x7)) & CT_MASK1;

            if (tmp_fp == (fp16 & FP_MASK1) && tmp_counter > 0) {
                half_flag = !half_flag;
                if (half_flag) {//0.5 prob to update
                    if (tmp_counter != 0x7F) {
                        *(uint32_t * )((uint8_t *) bucket + ((j + FP_LEN1) >> 3)) +=
                                ((uint32_t) 1) << ((j + FP_LEN1) & 0x7);
                    } else {
                        Switch(bucket, 2, num_lv_1, num_lv_2, num_lv_3, fp16, j);
                    }
                }
                goto pkt_done;
            }
        }


        if (num_lv_1 > 0) {
            uint16_t min_counter = -1;
            uint16_t min_index = -1;
            findMinCell(bucket, 1, num_lv_1, num_lv_2, num_lv_3, min_counter, min_index);
            if (min_counter == 0) {//find empty cell
                half_flag = !half_flag;
                if (half_flag) {
                    *(uint32_t * )((uint8_t *) bucket + (min_index >> 3)) |=
                            ((uint32_t) fp16 & FP_MASK1) << (min_index & 0x7);
                    *(uint32_t * )((uint8_t *) bucket + ((min_index + FP_LEN1) >> 3)) |=
                            ((uint32_t) 1) << ((min_index + FP_LEN1) & 0x7);
                }

            } else {//exp decay
                ranf = 1.0 * rand() / RAND_MAX;
                if (ranf < 0.5 * pow(b, log2(min_counter * 2) * -1)) {
                    if (min_counter == 1) {//replace fp
                        *(uint32_t * )((uint8_t *) bucket + (min_index >> 3)) &= ~(((uint32_t) FP_MASK1)
                                << (min_index & 0x7));
                        *(uint32_t * )((uint8_t *) bucket + (min_index >> 3)) |=
                                ((uint32_t) fp16 & FP_MASK1) << (min_index & 0x7);
                    } else {//decay counter
                        *(uint32_t * )((uint8_t *) bucket + ((min_index + FP_LEN1) >> 3)) -=
                                ((uint32_t) 1) << ((min_index + FP_LEN1) & 0x7);
                    }
                }
            }
        } else if (num_lv_2 > 0) {
            uint16_t min_counter = -1;
            uint16_t min_index = -1;
            findMinCell(bucket, 2, num_lv_1, num_lv_2, num_lv_3, min_counter, min_index);
            if (min_counter == 0) {//find empty cell
                *(uint32_t * )((uint8_t *) bucket + (min_index >> 3)) |=
                        ((uint32_t) fp16 & FP_MASK2) << (min_index & 0x7);
                *(uint32_t * )((uint8_t *) bucket + ((min_index + FP_LEN2) >> 3)) |=
                        ((uint32_t) 1) << ((min_index + FP_LEN2) & 0x7);
            } else {//exp decay
                ranf = 1.0 * rand() / RAND_MAX;
                if (ranf < pow(b, log2(min_counter) * -1)) {
                    if (min_counter == 1) {//replace fp
                        *(uint32_t * )((uint8_t *) bucket + (min_index >> 3)) &= ~(((uint32_t) FP_MASK2)
                                << (min_index & 0x7));
                        *(uint32_t * )((uint8_t *) bucket + (min_index >> 3)) |=
                                ((uint32_t) fp16 & FP_MASK2) << (min_index & 0x7);
                    } else {//decay counter
                        *(uint32_t * )((uint8_t *) bucket + ((min_index + FP_LEN2) >> 3)) -=
                                ((uint32_t) 1) << ((min_index + FP_LEN2) & 0x7);
                    }
                }
            }
        } else {
            uint16_t min_counter = EXP_MODE_T;
            uint16_t min_index = -1;
            findMinCell(bucket, 3, num_lv_1, num_lv_2, num_lv_3, min_counter, min_index);
            if (min_counter == 0) {//find empty cell
                *(uint32_t * )((uint8_t *) bucket + (min_index >> 3)) |=
                        ((uint32_t) fp16 & FP_MASK3) << (min_index & 0x7);
                *(uint32_t * )((uint8_t *) bucket + ((min_index + FP_LEN3) >> 3)) |=
                        ((uint32_t) 1) << ((min_index + FP_LEN3) & 0x7);
            } else if (min_counter < EXP_MODE_T) {//exp decay
                ranf = 1.0 * rand() / RAND_MAX;
                if (ranf < pow(b, log2(min_counter) * -1)) {
                    if (min_counter == 1) {//replace fp
                        *(uint32_t * )((uint8_t *) bucket + (min_index >> 3)) &= ~(((uint32_t) FP_MASK3)
                                << (min_index & 0x7));
                        *(uint32_t * )((uint8_t *) bucket + (min_index >> 3)) |=
                                ((uint32_t) fp16 & FP_MASK3) << (min_index & 0x7);
                    } else {//decay counter
                        *(uint32_t * )((uint8_t *) bucket + ((min_index + FP_LEN3) >> 3)) -=
                                ((uint32_t) 1) << ((min_index + FP_LEN3) & 0x7);
                    }
                }
            }
        }
        pkt_done:;
    }
}

uint32_t query(uint8_t *key) {
    uint16_t tmp_fp, tmp_counter;

    int num_lv_1, num_lv_2, num_lv_3;
    int start_lv2, start_lv1, end_lv1;

    int j;

    uint32_t hash = 0;

    hash = BKDRHash(key, KEY_SIZE);
    uint16_t bucket_index = hash % BUCKET_NUM;         //get bucket index
    uint16_t fp16 = finger_print(hash);                //get fp
    uint64_t *bucket = B[bucket_index];

    uint8_t meta = bucket[0] & META_MASK;
    metaCodeToData(meta, num_lv_1, num_lv_2, num_lv_3);

    //each 256-bit bucket is composed of 4 64-bit words, all cells are stored from the lower addresses. In each cell, lower bits are the fingerprint, and higher bits are the counter.
    start_lv2 = META_LENGTH + num_lv_3 * CELL_LEN3;
    start_lv1 = start_lv2 + num_lv_2 * CELL_LEN2;
    end_lv1 = start_lv1 + num_lv_1 * CELL_LEN1;

    //if exists a flow in level_3
    for (j = META_LENGTH; j < start_lv2; j += CELL_LEN3) {
        tmp_fp = ((*(uint32_t * )((uint8_t *) bucket + (j >> 3))) >> (j & 0x7)) & FP_MASK3;
        tmp_counter = ((*(uint32_t * )((uint8_t *) bucket + ((j + FP_LEN3) >> 3))) >> ((j + FP_LEN3) & 0x7)) & CT_MASK3;

        if (tmp_fp == fp16 && tmp_counter > 0) {
            if (tmp_counter <= EXP_MODE_MASK) {
                return tmp_counter;
            } else//exponential mode
            {
                return ((uint32_t) tmp_counter >> 4) * ((uint32_t) 1 << (4 + (tmp_counter & 0xF)));
            }
        }
    }
    //change when bucket structure changes
    //if existing flow in level_2
    for (j = start_lv2; j < start_lv1; j += CELL_LEN2) {
        tmp_fp = ((*(uint32_t * )((uint8_t *) bucket + (j >> 3))) >> (j & 0x7)) & FP_MASK2;
        tmp_counter = ((*(uint32_t * )((uint8_t *) bucket + ((j + FP_LEN2) >> 3))) >> ((j + FP_LEN2) & 0x7)) & CT_MASK2;

        if (tmp_fp == (fp16 & FP_MASK2) && tmp_counter > 0) {
            return tmp_counter;
        }
    }

    //if existing flow in level_1
    for (j = start_lv1; j < end_lv1; j += CELL_LEN1) {
        tmp_fp = ((*(uint16_t * )((uint8_t *) bucket + (j >> 3))) >> (j & 0x7)) & FP_MASK1;
        tmp_counter = ((*(uint16_t * )((uint8_t *) bucket + ((j + FP_LEN1) >> 3))) >> ((j + FP_LEN1) & 0x7)) & CT_MASK1;

        if (tmp_fp == (fp16 & FP_MASK1) && tmp_counter > 0) {
            return ((uint32_t) tmp_counter) * 2;
        }
    }

    return 0;
}

//just use the keys in actualFlowSizesVector as the known key set, the size info is ignored
void getEstimatedFlowSizes(double epsilon, vector <pair<uint8_t *, unsigned int>> actualFlowSizesVector,
                           unordered_map<uint8_t *, unsigned int, HashFunc, CmpFunc> &estimatedFlowSizes) {
    for (int i = 0; i < actualFlowSizesVector.size(); i++) {
        uint8_t *key = actualFlowSizesVector[i].first;

        uint32_t result1 = query(key);
        if (result1 >= THRESHOLD) {
            estimatedFlowSizes[key] = result1;
        }
    }
}

uint32_t
ReadInTraces(const char *traceFilePath, unordered_map<uint8_t *, uint32_t, HashFunc, CmpFunc> &actualFlowSizes) {
    FILE *fin = fopen(traceFilePath, "rb");

    uint8_t temp[KEY_SIZE]{0};
    uint8_t temp2[9]{0};

    uint32_t count = 0;
    uint8_t *key;
    while (fread(temp, 1, KEY_SIZE, fin) == KEY_SIZE) {
        key = (uint8_t *) malloc(KEY_SIZE);
        memcpy(key, temp, KEY_SIZE);
        keys[count] = key;
        if (actualFlowSizes.find(key) == actualFlowSizes.end()) {
            actualFlowSizes[key] = 1;
        } else {
            actualFlowSizes[key] += 1;
        }
        count++;

        if (count % 5000000 == 0) {
            printf("Successfully read in %s, %u items\n", traceFilePath, count);
            //            break;
        }
        if (fread(temp2, 1, 9, fin) != 9) {
           break;
       }
    }
    printf("Total Successfully read in %s, %u items\n", traceFilePath, count);
    fclose(fin);
    return count;
}

int main() {
    srand(4524524);
    long double totalPR = 0;
    long double totalRecall = 0;
    long double totalARE = 0;
    long double totalAAE = 0;
    long double totalTh = 0;

    long double totalPR_topK = 0;
    long double totalARE_topK = 0;
    long double totalAAE_topK = 0;

    unordered_map < uint8_t * , unsigned int, HashFunc, CmpFunc > actualFlowSizes;//count the actual flow sizes
    uint32_t packet_cnt = ReadInTraces("../data/20190117-130000-new.dat", actualFlowSizes);
    cout << "packet_cnt: " << packet_cnt << endl;

    const int testNum = 10;
    for (int i = 0; i < testNum; i++) {
//        srand((unsigned)time(NULL));
        // timespec time1, time2;
        __uint128_t resns;

        //change when bucket structure changes
        for (int j = 0; j < BUCKET_NUM; j++)//reset buckets
        {
            B[j][0] = 0;
            B[j][1] = 0;
        }

        clock_t time1 = clock();
        insert(packet_cnt);
        clock_t time2 = clock();

        // resns = (__uint128_t) (time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);

        // long double throughput = (double) 1000.0 * packet_cnt / resns;
        double numOfSeconds = ((double)time2 - time1) / CLOCKS_PER_SEC;//the seconds using to insert items
	    double throughput = (packet_cnt / 1000000.0) / numOfSeconds;

        vector <pair<uint8_t *, unsigned int>> actualFlowSizesVector;
        for (auto iter = actualFlowSizes.begin(); iter != actualFlowSizes.end(); iter++) {
            actualFlowSizesVector.push_back(make_pair(iter->first, iter->second));
        }
        sort(actualFlowSizesVector.begin(), actualFlowSizesVector.end(), cmpPairFunc);

        double epsilon = 0.6;
        vector <pair<uint8_t *, unsigned int>> estimatedFlowSizesVector;
        unordered_map < uint8_t * , unsigned int, HashFunc, CmpFunc > estimatedFlowSizes;
        getEstimatedFlowSizes(epsilon, actualFlowSizesVector, estimatedFlowSizes);

        for (auto iter = estimatedFlowSizes.begin(); iter != estimatedFlowSizes.end(); iter++) {
            estimatedFlowSizesVector.push_back(make_pair(iter->first, iter->second));
        }
        sort(estimatedFlowSizesVector.begin(), estimatedFlowSizesVector.end(), cmpPairFunc);

        //get acutal heavyHitters
        unordered_map < uint8_t * , unsigned int, HashFunc, CmpFunc > actualHeavyHitterFlowSizes;
        for (unsigned int idx = 0; idx < actualFlowSizesVector.size(); idx++) {
            unsigned int actualSize = actualFlowSizesVector[idx].second;
            if (actualSize >= THRESHOLD) {
                actualHeavyHitterFlowSizes[actualFlowSizesVector[idx].first] = actualSize;
            } else {
                break;
            }
        }

        //get actual topKFlows
        uint32_t k = 1024;
        unordered_map < uint8_t * , unsigned int, HashFunc, CmpFunc > actualTopKFlowSizes;
        for (unsigned int idx = 0; idx < k; idx++) {
            unsigned int actualSize = actualFlowSizesVector[idx].second;
            actualTopKFlowSizes[actualFlowSizesVector[idx].first] = actualSize;
        }
        unsigned int TPNum = 0;//True positive num
        unsigned int FPNum = 0;

        for (unsigned int idx = 0; idx < estimatedFlowSizesVector.size(); idx++) {
            uint8_t *key = estimatedFlowSizesVector[idx].first;
            if (actualHeavyHitterFlowSizes.find(key) != actualHeavyHitterFlowSizes.end()) {
                TPNum++;
            } else {
                FPNum++;
            }
        }

        ofstream report("./switchsketch128-noDoubleCheck -Report.txt");
        double AAEOfHH = 0;
        double AREOfHH = 0;

        double AAEOfTopK = 0;
        double AREOfTopK = 0;
        uint32_t TPNumOfTopK = 0;
        for (unsigned int idx = 0; idx < estimatedFlowSizesVector.size(); idx++) {

            uint8_t *key = estimatedFlowSizesVector[idx].first;
            uint32_t estimatedSize = estimatedFlowSizesVector[idx].second;

            unsigned int actualSize = 0;
            if (actualFlowSizes.find(key) != actualFlowSizes.end()) {
                actualSize = actualFlowSizes[key];
            }

            AAEOfHH += abs((double) estimatedSize - actualSize);
            double rE = abs((double) estimatedSize - actualSize) / actualSize;
            AREOfHH += rE;

            if (idx == k - 1) {
                AAEOfTopK = AAEOfHH / k;
                AREOfTopK = AREOfHH / k;
            }
            if (idx < k) {
                if (actualTopKFlowSizes.find(key) != actualTopKFlowSizes.end()) {
                    TPNumOfTopK++;
                }
            }
            if (actualSize < THRESHOLD) {
                report << "++++";
            }
            report << "Rep-" << idx << "\t" << estimatedSize << "\t" << actualSize << "\t"
                   << setiosflags(ios::fixed | ios::right) << setprecision(6) << AAEOfHH / (idx + 1) << "\t"
                   << AREOfHH / (idx + 1) << endl;
        }
        report.close();

        AAEOfHH = AAEOfHH / (TPNum + FPNum);
        AREOfHH = AREOfHH / (TPNum + FPNum);
        double precision = (double) TPNum / (TPNum + FPNum);
        double precision_topK = (double) TPNumOfTopK / k;

        uint32_t FNNum = actualHeavyHitterFlowSizes.size() - TPNum;
        double recall = (double) TPNum / actualHeavyHitterFlowSizes.size();

        ofstream out("./switchsketch128-noDoubleCheck -m64 -O.txt", ios::app);
        out << "****************************-----test " << i << endl;
        out << "get " << estimatedFlowSizes.size() << " flows" << endl;
        out << "### Heavy Hitter metrics: " << endl;
        out << "TP=" << TPNum << "\nFP=" << FPNum << "\nFN=" << FNNum << endl;

        out << "Recall=" << TPNum << "/" << actualHeavyHitterFlowSizes.size() << "=" << recall << endl;
        out << "PR=" << TPNum << "/" << (TPNum + FPNum) << "=" << precision << endl;
        out << "ARE=" << AREOfHH << "\nAAE=" << AAEOfHH << endl;
        out << "Th=" << throughput << " mps\n\n" << endl;
        out << "### TopK metrics: " << endl;
        out << "topK PR=" << TPNumOfTopK << "/" << k << "=" << precision_topK << endl;
        out << "topK ARE=" << AREOfTopK << "\nAAE=" << AAEOfTopK << endl;
        out.close();

        totalPR += precision;
        totalARE += AREOfHH;
        totalAAE += AAEOfHH;
        totalTh += throughput;
        totalRecall += recall;

        totalPR_topK += precision_topK;
        totalARE_topK += AREOfTopK;
        totalAAE_topK += AAEOfTopK;

        ofstream real("./switchsketch128-noDoubleCheck -Real.txt");
        AAEOfHH = 0;
        AREOfHH = 0;
        for (unsigned int idx = 0; idx < actualFlowSizesVector.size(); idx++) {
            uint8_t *key = actualFlowSizesVector[idx].first;
            unsigned int actualSize = actualFlowSizesVector[idx].second;
            if (actualSize >= THRESHOLD) {
                unsigned int estimatedSize = query(key);

                AAEOfHH += abs((double) estimatedSize - actualSize);
                double rE = abs((double) estimatedSize - actualSize) / actualSize;
                AREOfHH += rE;
                if (estimatedSize < THRESHOLD) {
                    real << "----";
                }
                real << "Real-" << idx << "\t" << estimatedSize << "\t" << actualSize << "\t"
                     << setiosflags(ios::fixed | ios::right) << setprecision(6) << AAEOfHH / (idx + 1) << "\t"
                     << AREOfHH / (idx + 1) << endl;
            } else {
                break;
            }
        }
        real.close();
    }

    ofstream out("./switchsketch128-noDoubleCheck -m64 -O.txt", ios::app);
    out << "################################" << endl;
    out << "### Heavy Hitter metrics: " << endl;
    out << "avg PR: " << totalPR / testNum << endl;
    out << "avg Recall: " << totalRecall / testNum << endl;
    out << "avg ARE: " << totalARE / testNum << endl;
    out << "avg AAE: " << totalAAE / testNum << endl;
    out << "avg Th: " << totalTh / testNum << endl;
    out << "### TopK metrics: " << endl;
    out << "topK PR=" << totalPR_topK / testNum << endl;
    out << "topK avg ARE: " << totalARE_topK / testNum << endl;
    out << "topK avg AAE: " << totalAAE_topK / testNum << endl;
    out.close();


    /* save the throughput data*/
    ofstream throughputFile("../Throughput.txt", ios::app);
    throughputFile << "SwitchSketch" << "\n" << totalTh / testNum << endl;

    /* free memory */
    for (int k = 0; k < packet_cnt; k++)
        free(keys[k]);
    free(keys);
}