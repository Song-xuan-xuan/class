#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STATES 100       // 最大状态数
#define MAX_NAME_LEN 20      // 状态名最大长度
#define MAX_TRANSITIONS 200  // 最大转换规则数
#define MAX_TERMINALS 20     // 最大终结符数
#define MAX_ROWS 100         // DFA表最大行数
#define MAX_LINE_LEN 256     // 输入行最大长度

// ==========================================
// 数据结构定义
// ==========================================

// 模拟 Python 的 Set (存储状态名集合)
typedef struct {
    char items[MAX_STATES][MAX_NAME_LEN];
    int count;
} StateSet;

// 存储 NFA 的一条转换规则
typedef struct {
    char src[MAX_NAME_LEN];
    char input_char; // '~' 表示空串
    char dst[MAX_NAME_LEN];
} Transition;

// 全局变量存储 NFA 信息
Transition nfa_transitions[MAX_TRANSITIONS];
int nfa_count = 0;

// 终结符集合
char terminals[MAX_TERMINALS];
int terminal_count = 0;

// DFA 表的一行
typedef struct {
    StateSet state_set;          // 当前状态集 (Total List 第一列)
    StateSet next_sets[MAX_TERMINALS]; // 经过各个终结符后的状态集
    char name[MAX_NAME_LEN];     // 最终编号名 (X, Y, 0, 1...)
} DFARow;

DFARow total_list[MAX_ROWS];
int total_rows = 0;

// ==========================================
// 辅助函数：集合操作
// ==========================================

// 比较函数，用于 qsort 排序字符串
int cmp_str(const void *a, const void *b) {
    return strcmp((const char *)a, (const char *)b);
}

// 比较函数，用于 qsort 排序字符
int cmp_char(const void *a, const void *b) {
    return (*(char *)a - *(char *)b);
}

// 初始化集合
void init_set(StateSet *set) {
    set->count = 0;
}

// 向集合添加元素 (去重)
void add_to_set(StateSet *set, const char *name) {
    for (int i = 0; i < set->count; i++) {
        if (strcmp(set->items[i], name) == 0) {
            return; // 已存在
        }
    }
    strcpy(set->items[set->count], name);
    set->count++;
}

// 排序集合 (为了比较两个集合是否相等)
void sort_set(StateSet *set) {
    qsort(set->items, set->count, MAX_NAME_LEN, cmp_str);
}

// 检查两个集合是否相等
int is_sets_equal(StateSet *a, StateSet *b) {
    if (a->count != b->count) return 0;
    // 假设都已排序
    for (int i = 0; i < a->count; i++) {
        if (strcmp(a->items[i], b->items[i]) != 0) return 0;
    }
    return 1;
}

// 检查某状态是否在集合中
int is_in_set(StateSet *set, const char *name) {
    for (int i = 0; i < set->count; i++) {
        if (strcmp(set->items[i], name) == 0) return 1;
    }
    return 0;
}

// 复制集合
void copy_set(StateSet *dest, StateSet *src) {
    dest->count = src->count;
    for (int i = 0; i < src->count; i++) {
        strcpy(dest->items[i], src->items[i]);
    }
}

// ==========================================
// 核心算法函数
// ==========================================

// Python: get_closure
void get_closure(StateSet *input_states, StateSet *result) {
    // 初始化 result = input_states
    copy_set(result, input_states);
    
    // 使用简单的栈/队列逻辑
    // 这里为了简化，采用循环检测直到集合大小不再增加 (类似 BFS)
    int changed = 1;
    while (changed) {
        changed = 0;
        int current_count = result->count;
        for (int i = 0; i < current_count; i++) {
            char *curr = result->items[i];
            
            // 查找 curr 经过 '~' 能到的状态
            for (int k = 0; k < nfa_count; k++) {
                if (strcmp(nfa_transitions[k].src, curr) == 0 && nfa_transitions[k].input_char == '~') {
                    char *next_state = nfa_transitions[k].dst;
                    if (!is_in_set(result, next_state)) {
                        add_to_set(result, next_state);
                        changed = 1;
                    }
                }
            }
        }
    }
    sort_set(result); // 对应 Python 的 sorted(list(closure))
}

// Python: move_set
void move_set(StateSet *states, char c, StateSet *result) {
    init_set(result);
    for (int i = 0; i < states->count; i++) {
        char *s = states->items[i];
        for (int k = 0; k < nfa_count; k++) {
            if (strcmp(nfa_transitions[k].src, s) == 0 && nfa_transitions[k].input_char == c) {
                add_to_set(result, nfa_transitions[k].dst);
            }
        }
    }
}

// 添加终结符 (去重)
void add_terminal(char c) {
    for (int i = 0; i < terminal_count; i++) {
        if (terminals[i] == c) return;
    }
    terminals[terminal_count++] = c;
}

// ==========================================
// 主函数
// ==========================================

int main() {

    char line[MAX_LINE_LEN];
    char start_node[] = "X";
    char final_symbol[] = "Y";

    // 1. 读取并解析输入
    while (1) {
        if (fgets(line, sizeof(line), stdin) == NULL) break;
        // 去除换行符
        line[strcspn(line, "\n")] = 0;
        if (strlen(line) == 0) break; // 空行结束

        // 解析 line
        // 格式: Src Src-Char->Dst 或 Src
        // 这是一个比较简单的解析，假设空格分隔
        char *token = strtok(line, " ");
        if (!token) continue;

        char src_state[MAX_NAME_LEN];
        strcpy(src_state, token);

        while ((token = strtok(NULL, " ")) != NULL) {
            // token 类似于 X-~->3
            char *arrow = strstr(token, "->");
            if (arrow) {
                // 提取 Dst
                char dst[MAX_NAME_LEN];
                strcpy(dst, arrow + 2); // 跳过 "->"

                // 提取 Char
                // token 开头到 arrow 之前是 Src-Char
                *arrow = '\0'; // 截断
                char *dash = strrchr(token, '-'); // 找最后一个 '-'
                if (dash) {
                    char c = *(dash + 1); // 字符
                    
                    // 存入 NFA
                    strcpy(nfa_transitions[nfa_count].src, src_state);
                    nfa_transitions[nfa_count].input_char = c;
                    strcpy(nfa_transitions[nfa_count].dst, dst);
                    nfa_count++;

                    if (c != '~') {
                        add_terminal(c);
                    }
                }
            }
        }
    }

    // 终结符排序
    qsort(terminals, terminal_count, sizeof(char), cmp_char);

    // ==========================================
    // 2. 构建 total_list
    // ==========================================
    
    // 初始状态 X 的闭包
    StateSet initial_set, initial_closure;
    init_set(&initial_set);
    add_to_set(&initial_set, start_node);
    
    get_closure(&initial_set, &initial_closure);

    // 加入 total_list 第一行
    total_list[0].state_set = initial_closure;
    total_rows = 1;

    int current_idx = 0;
    while (current_idx < total_rows) {
        StateSet *current_state_set = &total_list[current_idx].state_set;

        // 遍历终结符
        for (int j = 0; j < terminal_count; j++) {
            char char_in = terminals[j];
            
            StateSet moved, next_closure;
            move_set(current_state_set, char_in, &moved);
            get_closure(&moved, &next_closure);

            // 保存结果到当前行
            total_list[current_idx].next_sets[j] = next_closure;

            // 检查是否是新状态
            if (next_closure.count > 0) {
                int exists = 0;
                for (int k = 0; k < total_rows; k++) {
                    if (is_sets_equal(&total_list[k].state_set, &next_closure)) {
                        exists = 1;
                        break;
                    }
                }

                if (!exists) {
                    total_list[total_rows].state_set = next_closure;
                    total_rows++;
                }
            }
        }
        current_idx++;
    }

    // ==========================================
    // 3. 命名与编号
    // ==========================================
    int normal_counter = 0;
    int y_counter = 0;

    for (int i = 0; i < total_rows; i++) {
        StateSet *s = &total_list[i].state_set;
        
        if (i == 0) {
            strcpy(total_list[i].name, "X");
        } else {
            int has_y = 0;
            for (int k = 0; k < s->count; k++) {
                if (strcmp(s->items[k], final_symbol) == 0) {
                    has_y = 1;
                    break;
                }
            }

            if (has_y) {
                if (y_counter == 0) strcpy(total_list[i].name, "Y");
                else sprintf(total_list[i].name, "Y%d", y_counter);
                y_counter++;
            } else {
                sprintf(total_list[i].name, "%d", normal_counter);
                normal_counter++;
            }
        }
    }

    // ==========================================
    // 4. 输出结果 (分类排序)
    // ==========================================
    
    // 临时存储输出行，为了分类输出
    char x_lines[MAX_ROWS][256]; int x_cnt = 0;
    char y_lines[MAX_ROWS][256]; int y_cnt = 0;
    char num_lines[MAX_ROWS][256]; int num_cnt = 0;



    for (int i = 0; i < total_rows; i++) {
        // 如果是空集，通常不输出（保持Python逻辑: if not src_set continue）
        if (total_list[i].state_set.count == 0) continue;

        char buffer[256];
        char *src_name = total_list[i].name;
        
        // 构建输出行
        strcpy(buffer, src_name);

        for (int j = 0; j < terminal_count; j++) {
            StateSet *dst_set = &total_list[i].next_sets[j];
            if (dst_set->count == 0) continue;

            // 找到目标集对应的名字
            char *dst_name = NULL;
            for (int k = 0; k < total_rows; k++) {
                if (is_sets_equal(&total_list[k].state_set, dst_set)) {
                    dst_name = total_list[k].name;
                    break;
                }
            }
            
            if (dst_name) {
                char temp[50];
                sprintf(temp, " %s-%c->%s", src_name, terminals[j], dst_name);
                strcat(buffer, temp);
            }
        }

        // 分类
        if (strcmp(src_name, "X") == 0) {
            strcpy(x_lines[x_cnt++], buffer);
        } else if (src_name[0] == 'Y') {
            strcpy(y_lines[y_cnt++], buffer);
        } else {
            strcpy(num_lines[num_cnt++], buffer);
        }
    }

    // 按顺序打印
    for(int i=0; i<x_cnt; i++) printf("%s\n", x_lines[i]);
    for(int i=0; i<y_cnt; i++) printf("%s\n", y_lines[i]);
    for(int i=0; i<num_cnt; i++) printf("%s\n", num_lines[i]);

    return 0;
}