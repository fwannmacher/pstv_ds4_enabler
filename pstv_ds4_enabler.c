#include <psp2/kernel/modulemgr.h>
#include <psp2kern/io/fcntl.h>
#include <psp2kern/io/stat.h>
#include <psp2kern/kernel/sysclib.h> 
#include <taihen.h>


typedef struct sfo_header_t {
  unsigned int magic;
  unsigned int version;
  unsigned int key_table_offset;
  unsigned int data_table_offset;
  unsigned int index_table_entries;
} __attribute__((packed)) sfo_header_t;

typedef struct sfo_index_table_entry_t {
  unsigned short key_offset;
  unsigned short param_fmt;
  unsigned int param_len;
  unsigned int param_max_len;
  unsigned int data_offset;
} __attribute__((packed)) sfo_index_table_entry_t;


static SceUID hook_id;
static tai_hook_ref_t hook_ref;


static int SceProcessmgrForKernel_patched(char* title_id, int flags, char* path, void* unknown)
{
    if((hook_ref < 0) || !(title_id))
        return 1;

    if((strlen(title_id)) >= 3 && (strncmp("PCS", title_id, 3) == 0))
    {
        char patch_param_sfo_path[38];
        snprintf(patch_param_sfo_path, sizeof(patch_param_sfo_path), "ux0:patch/%.9s/sce_sys/param.sfo", title_id);
        char app_param_sfo_path[36];
        snprintf(app_param_sfo_path, sizeof(app_param_sfo_path), "ux0:app/%.9s/sce_sys/param.sfo", title_id);

        SceUID fd = ksceIoOpen(patch_param_sfo_path, SCE_O_RDONLY, 0777);

        if(fd < 0)
        {
            fd = ksceIoOpen(app_param_sfo_path, SCE_O_RDONLY, 0777);

            if(fd < 0)
                return 1;
        }

        SceIoStat stat;

        if(ksceIoGetstatByFd(fd, &stat) < 0)
            return 1;

        char param_sfo_content[4096];
        int bytes_read = ksceIoRead(fd, param_sfo_content, stat.st_size);

        if(ksceIoClose(fd) < 0)
            return 1;

        sfo_header_t* header = (sfo_header_t*)param_sfo_content;
        sfo_index_table_entry_t* entry = (sfo_index_table_entry_t*)(param_sfo_content + sizeof(sfo_header_t) + sizeof(sfo_index_table_entry_t) * 2);
        char* key = (char*)(param_sfo_content + header->key_table_offset + entry->key_offset);
        unsigned int* data = (unsigned int*)(param_sfo_content + header->data_table_offset + entry->data_offset);
        
        if(!strcmp(key, "ATTRIBUTE2") && !((*data) & 0x1))
        {
            (*data) += 1;

            char repatch_game_dir[22];
            char repatch_sce_sys_dir[30];
            char repatch_param_sfo[40];

            snprintf(repatch_game_dir, sizeof(repatch_game_dir), "ux0:rePatch/%s", title_id);
            ksceIoMkdir(repatch_game_dir, 0777);

            snprintf(repatch_sce_sys_dir, sizeof(repatch_sce_sys_dir), "%s/sce_sys", repatch_game_dir);
            ksceIoMkdir(repatch_sce_sys_dir, 0777);

            snprintf(repatch_param_sfo, sizeof(repatch_param_sfo), "%s/param.sfo", repatch_sce_sys_dir);
            fd = ksceIoOpen(repatch_param_sfo, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);

            if(fd < 0)
                return 1;

            ksceIoWrite(fd, param_sfo_content, bytes_read);

            if(ksceIoClose(fd) < 0)
                return 1;
        }
    }

    return TAI_CONTINUE(int, hook_ref, title_id, flags, path, unknown);
}

void _start() __attribute__ ((weak, alias("module_start")));

int module_start(SceSize argc, const void *args)
{
    hook_id = taiHookFunctionExportForKernel(
        KERNEL_PID,
        &hook_ref,
        "SceProcessmgr",
        0x7A69DE86,
        0x71CF71FD,
        SceProcessmgrForKernel_patched);

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args)
{
    if(hook_id >= 0)
        taiHookReleaseForKernel(hook_id, hook_ref);

    return SCE_KERNEL_STOP_SUCCESS;
}
