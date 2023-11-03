/******************************************************************************/
/* Important Fall 2023 CSCI 402 usage information:                            */
/*                                                                            */
/* This fils is part of CSCI 402 kernel programming assignments at USC.       */
/*         53616c7465645f5fd1e93dbf35cbffa3aef28f8c01d8cf2ffc51ef62b26a       */
/*         f9bda5a68e5ed8c972b17bab0f42e24b19daa7bd408305b1f7bd6c7208c1       */
/*         0e36230e913039b3046dd5fd0ba706a624d33dbaa4d6aab02c82fe09f561       */
/*         01b0fd977b0051f0b0ce0c69f7db857b1b5e007be2db6d42894bf93de848       */
/*         806d9152bd5715e9                                                   */
/* Please understand that you are NOT permitted to distribute or publically   */
/*         display a copy of this file (or ANY PART of it) for any reason.    */
/* If anyone (including your prospective employer) asks you to post the code, */
/*         you must inform them that you do NOT have permissions to do so.    */
/* You are also NOT permitted to remove or alter this comment block.          */
/* If this comment block is removed or altered in a submitted file, 20 points */
/*         will be deducted.                                                  */
/******************************************************************************/

#include "kernel.h"
#include "globals.h"
#include "types.h"
#include "errno.h"

#include "util/string.h"
#include "util/printf.h"
#include "util/debug.h"

#include "fs/dirent.h"
#include "fs/fcntl.h"
#include "fs/stat.h"
#include "fs/vfs.h"
#include "fs/vnode.h"

/* This takes a base 'dir', a 'name', its 'len', and a result vnode.
 * Most of the work should be done by the vnode's implementation
 * specific lookup() function.
 *
 * If dir has no lookup(), return -ENOTDIR.
 *
 * Note: returns with the vnode refcount on *result incremented.
 */
int
lookup(vnode_t *dir, const char *name, size_t len, vnode_t **result)
{
        //NOT_YET_IMPLEMENTED("VFS: lookup");
        //dbg(DBG_VFS,"VFS: Enter lookup(), look for %s, length %d\n", name, len);
        //preconditions:
        KASSERT(NULL != dir);
        KASSERT(NULL != name);
        KASSERT(NULL != result);
        if(dir->vn_ops->lookup==NULL)
        {
            //dbg(DBG_VFS,"VFS: Leave lookup(), return error ENOTDIR\n");
            return -ENOTDIR;
        }
        // else if(len>STR_MAX)
        // {
        //     dbg(DBG_VFS,"VFS: Leave lookup(), return error ENAMETOOLONG\n");
        //     return -ENAMETOOLONG;
        // }
        else
        {
            int val = dir->vn_ops->lookup(dir,name,len,result);
        //     dbg(DBG_VFS,"VFS: Leave lookup(), find %s, error=%d\n", name, ret);
            return val;
        }
        return 0;
}


/* When successful this function returns data in the following "out"-arguments:
 *  o res_vnode: the vnode of the parent directory of "name"
 *  o name: the `basename' (the element of the pathname)
 *  o namelen: the length of the basename
 *
 * For example: dir_namev("/s5fs/bin/ls", &namelen, &name, NULL,
 * &res_vnode) would put 2 in namelen, "ls" in name, and a pointer to the
 * vnode corresponding to "/s5fs/bin" in res_vnode.
 *
 * The "base" argument defines where we start resolving the path from:
 * A base value of NULL means to use the process's current working directory,
 * curproc->p_cwd.  If pathname[0] == '/', ignore base and start with
 * vfs_root_vn.  dir_namev() should call lookup() to take care of resolving each
 * piece of the pathname.
 *
 * Note: A successful call to this causes vnode refcount on *res_vnode to
 * be incremented.
 */
int
dir_namev(const char *pathname, size_t *namelen, const char **name,
          vnode_t *base, vnode_t **res_vnode)
{
        //NOT_YET_IMPLEMENTED("VFS: dir_namev");
        //preconditions:
        KASSERT(NULL != pathname);
        KASSERT(NULL != namelen);
        KASSERT(NULL != name);
        KASSERT(NULL != res_vnode);
        dbg(DBG_PRINT,"(GRADING2A 2.b)\n");

        vnode_t *start_dir = NULL;
        start_dir = base == NULL ? curproc->p_cwd : base;

        const char *start_ptr = pathname; 
        const char *slash_ptr = pathname;
        if(pathname[0] == '/'){
                start_dir = vfs_root_vn;
                start_ptr++;
        }

        vref(start_dir);

        //logic difference, verify
        //no vput?
        // if(strlen(pathname) == 1 && pathname[0] == '/'){
        //         *namelen = 0;
        //         *name = &pathname[1];
        //         *res_vnode = start_dir;
        //         return 0;
        // }

        vnode_t *next_dir = NULL;

        while (*start_ptr != '\0') {
                slash_ptr = strchr(start_ptr, '/');
                // if no more /'s, must be the end of the pathname ending with \0, fetch file details and return
                if (slash_ptr == NULL) {
                        *name = start_ptr;
                        *namelen = strlen(start_ptr);
                        *res_vnode = start_dir;
                        return 0;
                }
                else if(slash_ptr == start_ptr){ // if multiple /'s in the path just continue
                        start_ptr++;
                        continue;
                }
                int cur_name_len = slash_ptr - start_ptr;
                //str_max check??
                //move ahead from the /
                slash_ptr++;

                int lookup_res = lookup(start_dir, start_ptr, cur_name_len, &next_dir);
                if(lookup_res != 0){
                        vput(start_dir);
                        return lookup_res;
                }
                vput(start_dir);
                start_dir = next_dir;
                start_ptr = slash_ptr;                
        }
        //if did not return from while, path ends with /
        // verify: vput?
                *namelen = 1;
                const char *cur_dir = ".";
                *name = cur_dir;
                *res_vnode = start_dir;
                return 0;
      
}

/* This returns in res_vnode the vnode requested by the other parameters.
 * It makes use of dir_namev and lookup to find the specified vnode (if it
 * exists).  flag is right out of the parameters to open(2); see
 * <weenix/fcntl.h>.  If the O_CREAT flag is specified and the file does
 * not exist, call create() in the parent directory vnode. However, if the
 * parent directory itself does not exist, this function should fail - in all
 * cases, no files or directories other than the one at the very end of the path
 * should be created.
 *
 * Note: Increments vnode refcount on *res_vnode.
 */
int
open_namev(const char *pathname, int flag, vnode_t **res_vnode, vnode_t *base)
{
        // NOT_YET_IMPLEMENTED("VFS: open_namev");
        const char *name = NULL;
        size_t namelen;
        vnode_t *parent_dir;
        //dbg(DBG_PRINT, "(GRADING2B)\n"); 
        int dirname_res = dir_namev(pathname, &namelen, &name, base, &parent_dir);
        if(dirname_res != 0){
                //dbg(DBG_PRINT, "(GRADING2B)\n"); 
                return dirname_res;
        }
        // KASSERT(NULL != name || 0 != namelen);
        // dbg(DBG_PRINT, "(GRADING2B)\n"); 
        int lookup_res =  lookup(parent_dir, name, namelen, res_vnode);
        //if lookup successful : not required? Double check during testing
        // if(!lookup_res){
        //         vput(parent_dir); //free 
        //         //dbg(DBG_PRINT, "(GRADING2B)\n");
        //         return 0;
        // }

        //if error is -ENOENT and the flag is O_CREAT, creating new vnode
        if (lookup_res == -ENOENT && (flag & O_CREAT))
        {
                // Verify that the create operation exists for the parent directory
                KASSERT(NULL != parent_dir->vn_ops->create);
                dbg(DBG_PRINT, "(GRADING2A 2.c)\n");
                dbg(DBG_PRINT, "(GRADING2B)\n"); 

                int create_res = parent_dir->vn_ops->create(parent_dir, name, namelen, res_vnode);
                vput(parent_dir);
                //dbg(DBG_PRINT, "(GRADING2B)\n"); 
                return create_res;
                
        }

        vput(parent_dir);
        // dbg(DBG_PRINT, "(GRADING2B)\n"); 
        return lookup_res;
}

#ifdef _GETCWD_
/* Finds the name of 'entry' in the directory 'dir'. The name is writen
 * to the given buffer. On success 0 is returned. If 'dir' does not
 * contain 'entry' then -ENOENT is returned. If the given buffer cannot
 * hold the result then it is filled with as many characters as possible
 * and a null terminator, -ERANGE is returned.
 *
 * Files can be uniquely identified within a file system by their
 * inode numbers. */
int
lookup_name(vnode_t *dir, vnode_t *entry, char *buf, size_t size)
{
        NOT_YET_IMPLEMENTED("GETCWD: lookup_name");
        return -ENOENT;
}


/* Used to find the absolute path of the directory 'dir'. Since
 * directories cannot have more than one link there is always
 * a unique solution. The path is writen to the given buffer.
 * On success 0 is returned. On error this function returns a
 * negative error code. See the man page for getcwd(3) for
 * possible errors. Even if an error code is returned the buffer
 * will be filled with a valid string which has some partial
 * information about the wanted path. */
ssize_t
lookup_dirpath(vnode_t *dir, char *buf, size_t osize)
{
        NOT_YET_IMPLEMENTED("GETCWD: lookup_dirpath");

        return -ENOENT;
}
#endif /* _GETCWD_ */