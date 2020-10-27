#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_USER_FORBIDDEN_MOUNTS_LENGTH 100

static int
read_forbidden_mounts (char **array,
                         unsigned int *ptr_length)
{
  char *user_forbidden_mounts[] = { "/mnt/cdrom", "/mnt/cdaudio", "/tmp", "/var", 
          "/dev/shm", "/mnt/sambadir1", "/mnt/sambadir2" };
  unsigned int user_forbidden_mounts_length = sizeof (user_forbidden_mounts) 
          / sizeof (user_forbidden_mounts[0]);
  
  if (user_forbidden_mounts_length > *ptr_length)
    return 1;
  
  for (unsigned int loop_i_mount = 0; loop_i_mount < user_forbidden_mounts_length; loop_i_mount++)
    array[loop_i_mount] = strdup (user_forbidden_mounts[loop_i_mount]);
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
          array[loop_i_volume] = strdup (concat);
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
}

int
main (void)
{
  update_volumes ();
  update_mounts ();
  return 0;
}
