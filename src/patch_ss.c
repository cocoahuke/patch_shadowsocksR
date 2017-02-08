//
//  patch_ss.c
//  patch_ss
//
//  Created by huke on 2/8/17.
//  Copyright (c) 2017 com.cocoahuke. All rights reserved.
//

#include <stdio.h>
#include <unistd.h>;
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <pwd.h>

static int check_if_include_already(char *filepath){
    struct mach_header *mh = NULL;
    char machoheader[4096];
    
    FILE *fp_open = fopen(filepath,"r");
    if(!fp_open){
        printf("file isn't exist\n");
        exit(1);
    }
    
    if(fread(machoheader,1,4096,fp_open)!=4096){
        printf("fread error\n");
        goto done;
    }
    
    
    mh = (struct mach_header*)machoheader;
    if(!mh->cputype&CPU_TYPE_X86_64||!mh->filetype&MH_EXECUTE)
        goto done;
    
    const uint32_t cmd_count = mh->ncmds;
    struct load_command *cmds = (struct load_command*)((char*)mh+sizeof(struct mach_header_64));
    struct load_command* cmd = cmds;
    for (uint32_t i = 0; i < cmd_count; ++i){
        
        switch (cmd->cmd) {
                
            case LC_LOAD_DYLIB:
            {
                struct dylib_command *dyb_cmd = (struct dylib_command*)cmd;
                char *name = &((char *)dyb_cmd)[dyb_cmd->dylib.name.offset];
                if(strstr(name,"hack_clock_gettime.dylib")){
                    fclose(fp_open);
                    return 1;
                }
            }
                
        }
        cmd = (struct load_command*)((char*)cmd + cmd->cmdsize);
    }
    
done:
    fclose(fp_open);
    return 0;
}

#define IS_64_BIT(x) ((x) == MH_MAGIC_64 || (x) == MH_CIGAM_64)
#define IS_LITTLE_ENDIAN(x) ((x) == FAT_CIGAM || (x) == MH_CIGAM_64 || (x) == MH_CIGAM)
#define SWAP32(x, magic) (IS_LITTLE_ENDIAN(magic)? OSSwapInt32(x): (x))

void insert_dylib(FILE *f, size_t header_offset, const char *dylib_path) {
    
    fseek(f, header_offset, SEEK_SET);
    
    struct mach_header mh;
    fread(&mh, sizeof(struct mach_header), 1, f);
    
    if(mh.magic != MH_MAGIC_64 && mh.magic != MH_CIGAM_64 && mh.magic != MH_MAGIC && mh.magic != MH_CIGAM) {
        printf("Unknown magic: 0x%x\n", mh.magic);
        return;
    }
    
    size_t commands_offset = header_offset + (IS_64_BIT(mh.magic)? sizeof(struct mach_header_64): sizeof(struct mach_header));
    
    size_t path_padding = 4;
    if(SWAP32(mh.cputype, mh.magic) == CPU_TYPE_ARM64) {
        path_padding = 8;
    }
    
    size_t dylib_path_len = strlen(dylib_path);
    size_t dylib_path_size = (dylib_path_len & ~(path_padding - 1)) + path_padding;
    uint32_t cmdsize = (uint32_t)(sizeof(struct dylib_command) + dylib_path_size);
    
    struct dylib_command dylib_command = {
        .cmd = LC_LOAD_DYLIB,
        .cmdsize = SWAP32(cmdsize, mh.magic),
        .dylib = {
            .name = SWAP32(sizeof(struct dylib_command), mh.magic),
            .timestamp = 0,
            .current_version = 0,
            .compatibility_version = 0
        }
    };
    
    uint32_t sizeofcmds = SWAP32(mh.sizeofcmds, mh.magic);
    
    fseek(f, commands_offset + sizeofcmds, SEEK_SET);
    char space[cmdsize];
    
    fread(&space, cmdsize, 1, f);
    
    int empty = 1;
    for(int i = 0; i < cmdsize; i++) {
        if(space[i] != 0) {
            empty = 0;
            break;
        }
    }
    
    fseek(f, -((long)cmdsize), SEEK_CUR);
    
    char *dylib_path_padded = calloc(dylib_path_size, 1);
    memcpy(dylib_path_padded, dylib_path, dylib_path_len);
    
    fwrite(&dylib_command, sizeof(dylib_command), 1, f);
    fwrite(dylib_path_padded, dylib_path_size, 1, f);
    
    free(dylib_path_padded);
    
    mh.ncmds = SWAP32(SWAP32(mh.ncmds, mh.magic) + 1, mh.magic);
    sizeofcmds += cmdsize;
    mh.sizeofcmds = SWAP32(sizeofcmds, mh.magic);
    
    fseek(f, header_offset, SEEK_SET);
    fwrite(&mh, sizeof(mh), 1, f);
}

int main(int argc, const char * argv[]) {
    char tmpstr[1024];
    char tmpstr2[1024];

    for(size_t i=strlen(argv[0]);i>0;i--){
        if(argv[0][i]=='/'){
            strncpy(tmpstr,argv[0],i);
            break;
        }
    }
    
    snprintf(tmpstr,1024,"%s/hack_clock_gettime.dylib",tmpstr);
    
    if(access(tmpstr, F_OK)){
        printf("please run under build directory\n");exit(1);
    }
    
    snprintf(tmpstr2,1024,"mkdir -p /usr/local/lib&&cp %s /usr/local/lib/",tmpstr);
    system(tmpstr2);
    
    struct passwd *rootusr = getpwuid(getuid());
    snprintf(tmpstr, 1024, "%s/Library/Application Support/ShadowsocksX-R/ss-local", rootusr->pw_dir);
    if(!access(tmpstr, F_OK)){
        bzero(tmpstr2,1024);
        if (readlink(tmpstr, tmpstr2, 1024 - 1) != -1){
            if(!check_if_include_already(tmpstr2)){
                FILE *f = fopen(tmpstr2,"r+");
                if(!f){
                    printf("%s is not able to open\n",tmpstr2);exit(1);
                }
                insert_dylib(f,0,"/usr/local/lib/hack_clock_gettime.dylib");
                printf("patched %s\n", tmpstr2);
                fclose(f);
            }
        }
    }
    
    char *in_app = "/Applications/ShadowsocksX-R.app/Contents/Resources/ss-local";
    if(!access(in_app, F_OK)){
        if(!check_if_include_already(in_app)){
            FILE *f = fopen(in_app,"r+");
            if(!f){
                printf("%s is not able to open\n",in_app);exit(1);
            }
            insert_dylib(f,0,"/usr/local/lib/hack_clock_gettime.dylib");
            printf("patched %s\n", in_app);
            fclose(f);
        }

    }
    
    printf("patched complete!\n");
    printf("Have a good day in china!\n");
    return 0;
}
