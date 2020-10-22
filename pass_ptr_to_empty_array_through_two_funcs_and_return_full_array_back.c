#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

static int
read_forbidden_mounts (char **array, 
                         unsigned int *length) 
{
  char *user_forbidden_mounts[] = { "/mnt/cdrom", "/mnt/cdaudio", "/tmp", "/var", "/dev/shm", "/mnt/sambadir1", "/mnt/sambadir2" };
  unsigned int capacity;
  length = &capacity;
  
  for (unsigned int loop_i_mount = 0; loop_i_mount < sizeof (user_forbidden_mounts) / sizeof (user_forbidden_mounts[0]); loop_i_mount++)
    {
      capacity = loop_i_mount+1;
      array = (char **) realloc (array, capacity * sizeof(char *));
      array[loop_i_mount] = malloc (capacity * sizeof(char));
      strcpy (array[loop_i_mount], user_forbidden_mounts[loop_i_mount]);
    }
    
  syslog (LOG_EMERG, "%s[%u]: read_forbidden_mounts successfully assigned all the values, length = %u, array[%u] = %s", __FILE__, __LINE__, *length, 6, array[6]);
  
  return 0;
}

static int
read_forbidden_volumes (char **array, 
                         unsigned int *length) 
{
  char *concat;
  syslog (LOG_EMERG, "%s[%u]: read_forbidden_volumes started", __FILE__, __LINE__);
  
  if (read_forbidden_mounts (array, length) == 0) 
    {
      syslog (LOG_EMERG, "%s[%u]: read_forbidden_volumes successfully read %u forbidden mounts, array[%u] = %s", __FILE__, __LINE__, *length, 6, array[6]);
      for (unsigned int loop_i_volume = 0; loop_i_volume < *length; loop_i_volume++)
        {
          strcat (strcpy (concat, array[loop_i_volume]), " changed");
          *(array)[loop_i_volume] = *concat;
        }
      syslog (LOG_EMERG, "%s[%u]: read_forbidden_volumes successfully assigned all the values, length = %u, array [%u] = %s", __FILE__, __LINE__, *length, 6, array[6]);
      return 0;
    }
  else
    {
      syslog (LOG_EMERG, "%s[%u]: read_forbidden_volumes failed to read forbidden mounts", __FILE__, __LINE__);
      return 1;
    }
}

static void
update_volumes (void)
{
  char **ptr_user_forbidden_volumes = malloc (1 * sizeof(char*));
  unsigned int user_forbidden_volumes_length = 0;	
  unsigned int *ptr_user_forbidden_volumes_length = &user_forbidden_volumes_length;
  
  printf ("update_volumes started\n");
  if (ptr_user_forbidden_volumes == NULL) { 
    printf ("Memory not allocated.\n"); 
    exit (0); 
  }
  
    if (read_forbidden_volumes (ptr_user_forbidden_volumes, ptr_user_forbidden_volumes_length) == 0)
      syslog (LOG_EMERG, "%s[%u]: update_volumes successfully read %u forbidden volumes", __FILE__, __LINE__, user_forbidden_volumes_length);
    else
      syslog (LOG_EMERG, "%s[%u]: update_volumes failed to read forbidden volumes", __FILE__, __LINE__);
  printf ("update_volumes ended\n");
}

static void
update_mounts (void)
{
  char **ptr_user_forbidden_mounts = malloc (1 * sizeof(char*));
  unsigned int user_forbidden_mounts_length = 0;	
  unsigned int *ptr_user_forbidden_mounts_length = &user_forbidden_mounts_length;

  printf ("update_mounts started\n");
  if (ptr_user_forbidden_mounts == NULL) { 
    printf ("Memory not allocated.\n"); 
    exit (0); 
  }

    if (read_forbidden_mounts (ptr_user_forbidden_mounts, ptr_user_forbidden_mounts_length) == 0)
      syslog (LOG_EMERG, "%s[%u]: update_mounts successfully read %u forbidden mounts", __FILE__, __LINE__, user_forbidden_mounts_length);
    else
      syslog (LOG_EMERG, "%s[%u]: update_mounts failed to read forbidden mounts", __FILE__, __LINE__);
  printf ("update_mounts ended\n");
}

int
main (void)
{
  openlog ("abcd", LOG_PID, LOG_USER);
  update_volumes ();
  update_mounts ();
  closelog ();
  printf ("main ended\n");
  return 0;
}
