#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mntent.h>

#define MAX_USER_FORBIDDEN_MOUNTS_LENGTH 100

static int
read_fstab (char **array,
                         unsigned int *ptr_length)
{
  char *read_file;
  FILE *file;
  struct mntent ent;
  char buf[1024];
  struct mntent *mntent;
  char *mount_path;
  unsigned int loop_i_mount = 0;

#ifndef HAVE_GETMNTENT_R
  
  *ptr_length = 0;
  // in glib: read_file = get_fstab_file ();
  read_file = "/etc/fstab";
  
  file = setmntent (read_file, "r");
  if (file == NULL)
    {
      return 1;
    }
  else
    {      
      while ((mntent = getmntent_r (file, &ent, buf, sizeof (buf))) != NULL)
        {
          if (hasmntopt (mntent, "x-gvfs-hide") != NULL)
            {
              mount_path = mntent->mnt_dir;
              if ((array[loop_i_mount] = strdup (mount_path)) == NULL)
                return 1;
              loop_i_mount++;
            }
        }
      
      endmntent (file);
      *ptr_length = loop_i_mount;
      
      return 0;
    }

#else
  return 1;
#endif
}

static int
read_forbidden_mounts (char **array,
                         unsigned int *ptr_length)
{
  unsigned int user_forbidden_mounts_length = MAX_USER_FORBIDDEN_MOUNTS_LENGTH;
  char *user_forbidden_mounts[user_forbidden_mounts_length];
  unsigned int *ptr_user_forbidden_mounts_length = &user_forbidden_mounts_length;
  
  if (read_fstab (user_forbidden_mounts, ptr_user_forbidden_mounts_length) != 0)
    return 1;
  
  if (user_forbidden_mounts_length > *ptr_length)
    return 1;
  
  for (unsigned int loop_i_mount = 0; loop_i_mount < user_forbidden_mounts_length; loop_i_mount++)
    {
      if ((array[loop_i_mount] = strdup (user_forbidden_mounts[loop_i_mount])) == NULL)
        return 1;
      free (user_forbidden_mounts[loop_i_mount]);
    }
  *ptr_length = user_forbidden_mounts_length;
  
  return 0;
}

static int
read_forbidden_volumes (char **array,
                         unsigned int *ptr_length) 
{
  char concat[255];
  unsigned int length;
  
  if (read_forbidden_mounts (array, ptr_length) == 0)
    {
      length = *ptr_length;
      for (unsigned int loop_i_volume = 0; loop_i_volume < length; loop_i_volume++)
        {
          // no checks for length because it's a stub
          strcat (strcpy (concat, array[loop_i_volume]), " changed");
          // without this `free` there is a leak
          free (array[loop_i_volume]);
          if ((array[loop_i_volume] = strdup (concat)) == NULL)
            return 1;
        }
      return 0;
    }
  else
    {
      return 1;
    }
}

static void
update_volumes (void)
{
  unsigned int user_forbidden_volumes_length = MAX_USER_FORBIDDEN_MOUNTS_LENGTH;
  char *user_forbidden_volumes[user_forbidden_volumes_length];
  unsigned int *ptr_user_forbidden_volumes_length = &user_forbidden_volumes_length;
  
  if (read_forbidden_volumes (user_forbidden_volumes, ptr_user_forbidden_volumes_length) == 0)
    for (unsigned int loop_i_volume = 0; loop_i_volume < user_forbidden_volumes_length; 
            loop_i_volume++)
      free (user_forbidden_volumes[loop_i_volume]);
  else
    return;
}

static void
update_mounts (void)
{
  unsigned int user_forbidden_mounts_length = MAX_USER_FORBIDDEN_MOUNTS_LENGTH;
  char *user_forbidden_mounts[user_forbidden_mounts_length];
  unsigned int *ptr_user_forbidden_mounts_length = &user_forbidden_mounts_length;
  
  if (read_forbidden_mounts (user_forbidden_mounts, ptr_user_forbidden_mounts_length) == 0)
    for (unsigned int loop_i_mount = 0; loop_i_mount < user_forbidden_mounts_length; 
            loop_i_mount++)
      free (user_forbidden_mounts[loop_i_mount]);
  else
    return;
}

int
main (void)
{
  update_volumes ();
  update_mounts ();
  return 0;
}
