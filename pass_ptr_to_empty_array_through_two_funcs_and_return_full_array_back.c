#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <mntent.h>

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
    {
      syslog (LOG_EMERG, "%s[%u]: read_forbidden_mounts failed to read %s", __FILE__, __LINE__, read_file);
      return 1;
    }
   
  while ((mntent = getmntent_r (file, &ent, buf, sizeof (buf))) != NULL)
    {
      if (hasmntopt (mntent, "x-gvfs-hide") != NULL)
        {
          mount_path = mntent->mnt_dir;
          if (loop_i_mount == max_ptr_length)
            {
              syslog (LOG_EMERG, "%s[%u]: read_forbidden_mounts found more than %d (maximum allowed) entries in %s;"
                     " all the entries starting with %s will NOT be used", 
                     __FILE__, __LINE__, max_ptr_length, read_file, mount_path);
              break;
            }
          if ((array[loop_i_mount] = strdup (mount_path)) == NULL)
            {
              syslog (LOG_EMERG, "%s[%u]: read_forbidden_mounts failed to allocate memory", 
                     __FILE__, __LINE__);
              failed_el_index = loop_i_mount;
              break;
            }
          syslog (LOG_EMERG, "%s[%u]: read_forbidden_mounts found directory %s", 
	         __FILE__, __LINE__, mount_path);
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
  syslog (LOG_EMERG, "%s[%u]: read_forbidden_mounts successfully found %u directories in %s", 
         __FILE__, __LINE__, *ptr_length, read_file);
  return 0;
#else
  syslog (LOG_EMERG, "%s[%u]: read_forbidden_mounts can't find getmntent_r", __FILE__, __LINE__);
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
  unsigned int last_el;// used in syslog only
  syslog (LOG_EMERG, "%s[%u]: read_forbidden_volumes started", __FILE__, __LINE__);
  
  if (read_forbidden_mounts (array, ptr_length) == 0)
    {
      length = *ptr_length;
      last_el = length - 1;
      syslog (LOG_EMERG, "%s[%u]: read_forbidden_volumes successfully read %u forbidden mounts, "
              "array[%u] = %s", __FILE__, __LINE__, length, last_el, array[last_el]);
      for (unsigned int loop_i_volume = 0; loop_i_volume < length; loop_i_volume++)
        {
          snprintf_result = snprintf (concat, sizeof (concat), "%s%s", array[loop_i_volume], " changed");
          if (snprintf_result >= sizeof (concat))
            {
              syslog (LOG_EMERG, "%s[%u]: read_forbidden_volumes truncated strings while concatenating; "
                      "in principle, here must be realloc + another snprintf, but there aren't because it's just a stub",
                      __FILE__, __LINE__);
            }
          free (array[loop_i_volume]);// without this `free` there is a leak
          if ((array[loop_i_volume] = strdup (concat)) == NULL)
            {
              syslog (LOG_EMERG, "%s[%u]: read_forbidden_volumes failed to allocate memory", 
                      __FILE__, __LINE__);
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
      
      syslog (LOG_EMERG, "%s[%u]: read_forbidden_volumes successfully assigned all the values, "
              "length = %u, array[%u] = %s", __FILE__, __LINE__, length, last_el, array[last_el]);
      return 0;
    }
  else
    {
      syslog (LOG_EMERG, "%s[%u]: read_forbidden_volumes failed to read forbidden mounts", 
              __FILE__, __LINE__);
      return 1;
    }
}

static void
update_volumes (void)
{
  char *user_forbidden_volumes[MAX_USER_FORBIDDEN_MOUNTS_LENGTH];
  unsigned int user_forbidden_volumes_length = sizeof (user_forbidden_volumes) / sizeof (user_forbidden_volumes[0]);
  unsigned int *ptr_user_forbidden_volumes_length = &user_forbidden_volumes_length;
  printf ("update_volumes started\n");
  
  if (read_forbidden_volumes (user_forbidden_volumes, ptr_user_forbidden_volumes_length) == 0)
    {
      syslog (LOG_EMERG, "%s[%u]: update_volumes successfully read %u forbidden volumes", 
              __FILE__, __LINE__, user_forbidden_volumes_length);
      for (unsigned int loop_i_volume = 0; loop_i_volume < user_forbidden_volumes_length; 
              loop_i_volume++)
        free (user_forbidden_volumes[loop_i_volume]);
    }
  else
    {
      syslog (LOG_EMERG, "%s[%u]: update_volumes failed to read forbidden volumes", 
              __FILE__, __LINE__);
      return;
    }
  
  printf ("update_volumes ended\n");
}

static void
update_mounts (void)
{
  char *user_forbidden_mounts[MAX_USER_FORBIDDEN_MOUNTS_LENGTH];
  unsigned int user_forbidden_mounts_length = sizeof (user_forbidden_mounts) / sizeof (user_forbidden_mounts[0]);
  unsigned int *ptr_user_forbidden_mounts_length = &user_forbidden_mounts_length;
  printf ("update_mounts started\n");
  
  if (read_forbidden_mounts (user_forbidden_mounts, ptr_user_forbidden_mounts_length) == 0)
    {
      syslog (LOG_EMERG, "%s[%u]: update_mounts successfully read %u forbidden mounts", 
              __FILE__, __LINE__, user_forbidden_mounts_length);
      for (unsigned int loop_i_mount = 0; loop_i_mount < user_forbidden_mounts_length; 
              loop_i_mount++)
        free (user_forbidden_mounts[loop_i_mount]);
    }
  else
    {
      syslog (LOG_EMERG, "%s[%u]: update_mounts failed to read forbidden mounts", 
              __FILE__, __LINE__);
      return;
    }
  
  printf ("update_mounts ended\n");
}

int
main (void)
{
  openlog ("pass_ptr", LOG_PID, LOG_USER);
  update_volumes ();
  update_mounts ();
  closelog ();
  printf ("main ended\n");
  return 0;
}
