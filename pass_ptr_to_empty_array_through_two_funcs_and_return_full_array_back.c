#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
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


  if (1 == 1) {// in glib: #ifdef HAVE_GETMNTENT_R
  
  *ptr_length = 0;
  // in glib: read_file = get_fstab_file ();
  read_file = "/etc/fstab";
  
  file = setmntent (read_file, "r");
  if (file == NULL)
    {
      syslog (LOG_EMERG, "%s[%u]: read_fstab failed to read %s", __FILE__, __LINE__, read_file);
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
                {
                  syslog (LOG_EMERG, "%s[%u]: read_fstab failed to allocate memory", 
                          __FILE__, __LINE__);
                  return 1;
                }
              syslog (LOG_EMERG, "%s[%u]: read_fstab found directory %s", 
	              __FILE__, __LINE__, mount_path);
              loop_i_mount++;
            }
        }
      
      endmntent (file);
      *ptr_length = loop_i_mount;
      
      syslog (LOG_EMERG, "%s[%u]: read_fstab successfully found %u directories in %s", 
              __FILE__, __LINE__, *ptr_length, read_file);
      return 0;
    }


}  else {// in glib: #else
  syslog (LOG_EMERG, "%s[%u]: read_fstab can't find getmntent_r", __FILE__, __LINE__);
  return 1;
}// in glib: #endif
}

static int
read_forbidden_mounts (char **array,
                         unsigned int *ptr_length)
{
  unsigned int user_forbidden_mounts_length = MAX_USER_FORBIDDEN_MOUNTS_LENGTH;
  char *user_forbidden_mounts[user_forbidden_mounts_length];
  unsigned int *ptr_user_forbidden_mounts_length = &user_forbidden_mounts_length;
  unsigned int last_el;// used in syslog only

  syslog (LOG_EMERG, "%s[%u]: read_forbidden_mounts started", __FILE__, __LINE__);

  if (read_fstab (user_forbidden_mounts, ptr_user_forbidden_mounts_length) != 0)
    {
      syslog (LOG_EMERG, "%s[%u]: read_forbidden_mounts can't continue", __FILE__, __LINE__);
      return 1;
    }
  
  if (user_forbidden_mounts_length > *ptr_length)
    {
      syslog (LOG_EMERG, "%s[%u]: read_forbidden_mounts can't assign values because "
              "user_forbidden_mounts contains %d entries (more than %d entries are not allowed)", 
              __FILE__, __LINE__, user_forbidden_mounts_length, *ptr_length);
      return 1;
    }
  
  for (unsigned int loop_i_mount = 0; loop_i_mount < user_forbidden_mounts_length; loop_i_mount++)
    {
      if ((array[loop_i_mount] = strdup (user_forbidden_mounts[loop_i_mount])) == NULL)
        {
          syslog (LOG_EMERG, "%s[%u]: read_forbidden_mounts failed to allocate memory", 
                  __FILE__, __LINE__);
          return 1;
        }
      free (user_forbidden_mounts[loop_i_mount]);
    }
  *ptr_length = user_forbidden_mounts_length;
  last_el = user_forbidden_mounts_length - 1;

  syslog (LOG_EMERG, "%s[%u]: read_forbidden_mounts successfully assigned all the values, "
          "length = %u, array[%u] = %s", __FILE__, __LINE__, *ptr_length, last_el, array[last_el]);
  return 0;
}

static int
read_forbidden_volumes (char **array,
                         unsigned int *ptr_length) 
{
  char concat[255];
  unsigned int length;
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
          // no checks for length because it's a stub
          strcat (strcpy (concat, array[loop_i_volume]), " changed");
          // without this `free` there is a leak
          free (array[loop_i_volume]);
          if ((array[loop_i_volume] = strdup (concat)) == NULL)
            {
              syslog (LOG_EMERG, "%s[%u]: read_forbidden_volumes failed to allocate memory", 
                      __FILE__, __LINE__);
              return 1;
            }
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
  unsigned int user_forbidden_volumes_length = MAX_USER_FORBIDDEN_MOUNTS_LENGTH;
  char *user_forbidden_volumes[user_forbidden_volumes_length];
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
  unsigned int user_forbidden_mounts_length = MAX_USER_FORBIDDEN_MOUNTS_LENGTH;
  char *user_forbidden_mounts[user_forbidden_mounts_length];
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
