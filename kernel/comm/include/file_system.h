#ifndef file_system__
#define file_system__

typedef struct file_system_s_node {
    char const* name;
} file_system_s_node;

typedef struct file_system_s_file_interface {
    int (*open)(file_system_s_node* file, char const* mode, char const* args);
    int (*close)(file_system_s_node* file);
    int (*read)(file_system_s_node* file, void* buff, int length);
    int (*write)(file_system_s_node* file, void const* buff, int length);
    int (*ctrl)(file_system_s_node* file, char const* cmd, void* data);
} file_system_s_file_interface;

typedef struct file_system_s_interface { // 如果name的最后一个字符为'/'，则表示这是一个目录的名字，否则这是一个文件的名字。
    int (*create)(void* param, file_system_s_node* parent, char const* name);
    int (*remove)(void* param, file_system_s_node* parent, file_system_s_node* node);

    file_system_s_node* (*parent)(file_system_s_node* node);
    file_system_s_node* (*child)(void* param, file_system_s_node const* dir, char const* name); // 如果dir为NULL，找根目录下的子目录。如果name为空，则直接返回根目录。
    void (*free)(void* param, file_system_s_node* node);

    file_system_s_file_interface const* (*interface)(void* param, file_system_s_node const* file);
} file_system_s_interface;

typedef struct file_system_s_result {
    file_system_s_node*            node;
    file_system_s_interface const* interface;
    void*                          param;
} file_system_s_result;

void file_system_init(void);

int file_system_mount(char const* path, struct file_system_s_interface const* interface, void* param);

int file_system_create(char const* path);

file_system_s_result file_system_find_dir(char const* path);

unsigned file_system_name_len(char const* path);
unsigned file_system_dir_len(char const* path);

#endif
