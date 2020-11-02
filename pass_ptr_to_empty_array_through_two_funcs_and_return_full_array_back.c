#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mntent.h>
#include <limits.h>

#define MAX_USER_FORBIDDEN_MOUNTS_LENGTH 100

static int
read_forbidden_mounts (char **array,
                         unsigned int *ptr_length)
{
#if 1// in glib: #ifdef HAVE_GETMNTENT_R
  const char *read_file;
  FILE *file;
  struct mntent ent;
  char buf[1024];
  struct mntent *mntent;
  char *mount_path;
  unsigned int loop_i_mount = 0;
  unsigned int failed_el_index = UINT_MAX;// if an element fails to allocate memory, this variable will contain its index (>=0)
  unsigned int max_ptr_length = *ptr_length;
  
  *ptr_length = 0;
  read_file = "/etc/fstab";// in glib: read_file = get_fstab_file ();
  
  file = setmntent (read_file, "r");
  if (file == NULL)
    return 1;
   
  while ((mntent = getmntent_r (file, &ent, buf, sizeof (buf))) != NULL)
    {
      if (hasmntopt (mntent, "x-gvfs-hide") != NULL)
        {
          mount_path = mntent->mnt_dir;
          if (loop_i_mount == max_ptr_length)
            break;
          if ((array[loop_i_mount] = strdup (mount_path)) == NULL)
            {
              failed_el_index = loop_i_mount;
              break;
            }
          loop_i_mount++;
        }
    }

  endmntent (file);

  if (failed_el_index != -1)
    {
      for (unsigned int loop_j = 0; loop_j < failed_el_index; loop_j++)
        free (array[loop_j]);
      return 1;
    }
  
  *ptr_length = loop_i_mount;
  return 0;
#else
  return 1;
#endif
}

static int
read_forbidden_volumes (char **array,
                         unsigned int *ptr_length) 
{
  unsigned int length;
  char concat[255];
  int snprintf_result;
  unsigned int failed_el_index = UINT_MAX;// if an element fails to allocate memory, this variable will contain its index (>=0)
  
  if (read_forbidden_mounts (array, ptr_length) == 0)
    {
      length = *ptr_length;
      for (unsigned int loop_i_volume = 0; loop_i_volume < length; loop_i_volume++)
        {
          snprintf_result = snprintf (concat, sizeof (concat), "%s%s", array[loop_i_volume], " changed");
          free (array[loop_i_volume]);// without this `free` there is a leak
          if ((array[loop_i_volume] = strdup (concat)) == NULL)
            {
              failed_el_index = loop_i_volume;
              break;
            }
        }
      
      if (failed_el_index != -1)
        {
          for (unsigned int loop_j = 0; loop_j <= failed_el_index; loop_j++)
            free (array[loop_j]);
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
  char *user_forbidden_volumes[MAX_USER_FORBIDDEN_MOUNTS_LENGTH];
  unsigned int user_forbidden_volumes_length = sizeof (user_forbidden_volumes) / sizeof (user_forbidden_volumes[0]);
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
  char *user_forbidden_mounts[MAX_USER_FORBIDDEN_MOUNTS_LENGTH];
  unsigned int user_forbidden_mounts_length = sizeof (user_forbidden_mounts) / sizeof (user_forbidden_mounts[0]);
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
