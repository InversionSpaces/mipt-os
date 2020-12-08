#include <errno.h>
#include <limits.h>
#include <sys/mman.h>
#include <unistd.h>

#include <memory.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define FUSE_USE_VERSION 30
#include <fuse3/fuse.h>

typedef struct {
    char name[NAME_MAX];
    char* data;
    size_t data_size;
} file_entry_t;

typedef struct {
    file_entry_t* files;
    size_t files_size;
} sfs_data_t;

typedef struct {
    char* name;
    char* map;
    size_t len;
    struct stat st;
} src_file_t;

static src_file_t src_file;
static sfs_data_t data;

static int init()
{
    memset(&data, 0, sizeof(data));

    int fd = open(src_file.name, O_RDONLY);
    if (fd == -1)
        return -1;

    if (fstat(fd, &(src_file.st)) == -1) {
        close(fd);

        return -1;
    }

    off_t start = lseek(fd, 0, SEEK_SET);
    off_t end = lseek(fd, 0, SEEK_END);
    src_file.len = end - start;

    src_file.map = mmap(NULL, src_file.len, PROT_READ, MAP_SHARED, fd, 0);

    close(fd);

    if (src_file.map == MAP_FAILED)
        return -1;

    sscanf(src_file.map, "%lu", &data.files_size);
    data.files = malloc(sizeof(file_entry_t) * data.files_size);

    if (data.files == NULL) {
        munmap(src_file.map, src_file.len);
        return -1;
    }

    char* pos = strchr(src_file.map, '\n') + 1;
    for (size_t i = 0; i < data.files_size; ++i) {
        sscanf(pos, "%s %lu", data.files[i].name, &data.files[i].data_size);
        pos = strchr(pos, '\n') + 1;
    }

    ++pos;

    for (size_t i = 0; i < data.files_size; ++i) {
        data.files[i].data = pos;
        pos += data.files[i].data_size;
    }

    return 0;
}

static int deinit()
{
    return munmap(src_file.map, src_file.len);
}

int sfs_stat(const char* path, struct stat* st, struct fuse_file_info* fi)
{
    if (strcmp("/", path) == 0) {
        st->st_mode = 0555 | S_IFDIR;
        st->st_nlink = 2;

        return 0;
    }

    const char* name = path + 1;

    for (size_t i = 0; i < data.files_size; ++i) {
        if (strcmp(name, data.files[i].name) != 0)
            continue;

        st->st_size = data.files[i].data_size;
        st->st_mode = 0444 | S_IFREG;
        st->st_nlink = 1;

        return 0;
    }

    return -ENOENT;
}

int sfs_readdir(
    const char* path,
    void* out,
    fuse_fill_dir_t filler,
    off_t off,
    struct fuse_file_info* fi,
    enum fuse_readdir_flags flags)
{
    if (strcmp("/", path) != 0)
        return -ENOENT;

    if (filler(out, ".", NULL, 0, 0) == 1)
        return 0;

    if (filler(out, "..", NULL, 0, 0) == 1)
        return 0;

    for (size_t i = 0; i < data.files_size; ++i) {
        if (filler(out, data.files[i].name, NULL, 0, 0) == 1)
            return 0;
    }

    return 0;
}

int sfs_open(const char* path, struct fuse_file_info* fi)
{
    return 0;
}

int sfs_read(
    const char* path,
    char* out,
    size_t size,
    off_t off,
    struct fuse_file_info* fi)
{
    const char* name = path + 1;
    for (size_t i = 0; i < data.files_size; ++i) {
        if (strcmp(name, data.files[i].name) != 0)
            continue;

        size_t file_size = data.files[i].data_size;
        if (off > file_size)
            return 0;

        size_t left = file_size - off;
        size_t result_size = size > left ? left : size;

        if (result_size > 0)
            memcpy(out, data.files[i].data, result_size);

        return result_size;
    }

    return -ENOENT;
}

struct fuse_operations fuse_ops = {
    .getattr = sfs_stat,
    .readdir = sfs_readdir,
    .open = sfs_open,
    .read = sfs_read,
};

int main(int argc, char* argv[])
{
    typedef struct {
        char* src_file;
    } sfs_opt_t;

    sfs_opt_t options;
    memset(&options, 0, sizeof(options));

    struct fuse_opt opt_specs[] = {
        {"--src %s", offsetof(sfs_opt_t, src_file), 0}, {NULL, 0, 0}};

    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    fuse_opt_parse(&args, &options, opt_specs, NULL);

    if (options.src_file == NULL) {
        perror("No source file given");
        return 1;
    }

    src_file.name = options.src_file;

    int res = init();
    if (res != 0)
        return res;

    res = fuse_main(args.argc, args.argv, &fuse_ops, NULL);
    deinit();

    return res;
}
