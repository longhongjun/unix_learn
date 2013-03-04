#define _XOPEN_SOURCE

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <ndbm.h>

#include "cd_data.h"

#define CDC_FILE_BASE "cdc_data"
#define CDT_FILE_BASE "cdt_data"
#define CDC_FILE_DIR  "cdc_data_dir"
#define CDC_FILE_PAG  "cdc_data.pag"
#define CDT_FILE_DIR  "cdt_data.dir"
#define CDT_FILE_PAG  "cdt_data.pag"

static DBM *cdc_dbm_ptr = NULL;
static DBM *cdt_dbm_ptr = NULL;

int database_initialize(const int new_database) {
  int open_mode = O_CREAT | O_RDWR;

  if (cdc_dbm_ptr) dbm_close(cdc_dbm_ptr);
  if (cdt_dbm_ptr) dbm_close(cdt_dbm_ptr);

  if (new_database) {
    unlink(CDC_FILE_PAG);
    unlink(CDC_FILE_DIR);
    unlink(CDT_FILE_PAG);
    unlink(CDT_FILE_DIR);
  }

  cdc_dbm_ptr = dbm_open(CDC_FILE_BASE, open_mode, 0644);
  cdt_dbm_ptr = dbm_open(CDT_FILE_BASE, open_mode, 0644);
  if (!cdc_dbm_ptr || !cdt_dbm_ptr) {
    fprintf(stderr, "Unalbe to create database\n");
    cdc_dbm_ptr = cdt_dbm_ptr = NULL;
    return 0;
  }

  return 1;
}

void database_close(void) {
  if (cdc_dbm_ptr) dbm_close(cdc_dbm_ptr);
  if (cdt_dbm_ptr) dbm_close(cdt_dbm_ptr);

  cdc_dbm_ptr = cdt_dbm_ptr = NULL;
}

cdc_entry get_cdc_entry(const char *cd_catalog_ptr) {
  cdc_entry entry_to_return;
  char entry_to_find[CAT_CAT_LEN + 1];
  datum local_data_datum;
  datum local_key_datum;

  memset(&entry_to_return, '\0', sizeof(entry_to_return));
  if (!cdc_dbm_ptr || !cdt_dbm_ptr)
    return entry_to_return;
  if (!cd_catalog_ptr) return entry_to_return;

  memset(&entry_to_find, '\0', sizeof(entry_to_find));
  strcpy(entry_to_find, cd_catalog_ptr);

  local_key_datum.dptr = entry_to_find;
  local_key_datum.dsize = sizeof(entry_to_find);

  memset(&local_data_datum, '\0', sizeof(local_data_datum));
  local_data_datum = dbm_fetch(cdc_dbm_ptr, local_key_datum);
  if (local_key_datum.dptr) {
    memcpy(&entry_to_return, (char *)local_data_datum.dptr,
        local_data_datum.dsize);
  }

  return entry_to_return;
}

cdt_entry get_cdt_entry(const char *cd_catalog_prt, const int track_no) {
  cdt_entry entry_to_return;
  char entry_to_find[CAT_CAT_LEN + 1];
  datum local_data_datum;
  datum local_key_datum;

  memset(&entry_to_return, '\0', sizeof(entry_to_return));

  if (!cdc_dbm_ptr || !cdt_dbm_ptr) return entry_to_return;
  if (!cd_catalog_prt) return entry_to_return;
  if (strlen(cd_catalog_prt) >= CAT_CAT_LEN) return (entry_to_return);

  memset(&entry_to_find, '\0', sizeof(entry_to_find));
  sprintf(entry_to_find, "%s %d", cd_catalog_prt, track_no);

  local_key_datum.dptr = entry_to_find;
  local_key_datum.dsize = sizeof(entry_to_find);

  memset(&local_data_datum, '\0', sizeof(local_data_datum));
  local_data_datum = dbm_fetch(cdt_dbm_ptr, local_key_datum);
  if (local_data_datum.dptr) {
    memcpy(&entry_to_return, (char *)local_data_datum.dptr,
        local_data_datum.dsize);
  }

  return (entry_to_return);
}

int add_cdc_entry(const cdc_entry entry_to_add) {
  char key_to_add[CAT_CAT_LEN + 1];
  datum local_key_datum;
  datum local_data_datum;
  int result;

  if (!cdc_dbm_ptr || !cdt_dbm_ptr) return 0;
  if (strlen(entry_to_add.catalog) >= CAT_CAT_LEN) return 0;

  memset(&key_to_add, '\0', sizeof(key_to_add));
  strcpy(key_to_add, entry_to_add.catalog);

  local_key_datum.dptr = key_to_add;
  local_key_datum.dsize = sizeof(key_to_add);
  local_data_datum.dptr = (char *)&entry_to_add;
  local_data_datum.dsize = sizeof(entry_to_add);

  result = dbm_store(cdc_dbm_ptr, local_key_datum, local_data_datum,
      DBM_REPLACE);
  if (result == 0)
    return 1;

  return 0;
}

int add_cdt_entry(const cdt_entry entry_to_add) {
  char key_to_add[CAT_CAT_LEN + 10];
  datum local_data_datum;
  datum local_key_datum;
  int result;

  if (!cdc_dbm_ptr || !cdt_dbm_ptr) return 0;
  if (strlen(entry_to_add.catalog) >= CAT_CAT_LEN) return 0;

  memset(&key_to_add, '\0', sizeof(key_to_add));
  sprintf(key_to_add, "%s %d", entry_to_add.catalog, entry_to_add.track_no);

  local_key_datum.dptr = key_to_add;
  local_key_datum.dsize = sizeof(key_to_add);
  local_data_datum.dptr = (char *)&entry_to_add;
  local_data_datum.dsize = sizeof(entry_to_add);

  result = dbm_store(cdt_dbm_ptr, local_key_datum, local_data_datum,
      DBM_REPLACE);

  if (result == 0)
    return 1;

  return 0;
}

int del_cdc_entry(const char *cd_catalog_prt) {
  char key_to_del[CAT_CAT_LEN + 1];
  datum local_key_datum;
  int result;

  if (!cdc_dbm_ptr || !cdt_dbm_ptr) return 0;
  if (strlen(cd_catalog_prt) >= CAT_CAT_LEN) return 0;

  memset(key_to_del, '\0', sizeof(key_to_del));
  strcpy(key_to_del, cd_catalog_prt);

  local_key_datum.dptr = key_to_del;
  local_key_datum.dsize = sizeof(key_to_del);

  result = dbm_delete(cdc_dbm_ptr, local_key_datum);
  if (result == 0)
    return 1;
  return 0;
}

int del_cdt_entry(const char *cd_catalog_prt, const int track_no) {
  char key_to_del[CAT_CAT_LEN + 10];
  datum local_key_datum;
  int result;

  if (!cdc_dbm_ptr || !cdt_dbm_ptr) return 0;
  if (strlen(cd_catalog_prt) >= CAT_CAT_LEN) return 0;

  memset(&key_to_del, '\0', sizeof(key_to_del));
  sprintf(key_to_del, "%s %d", cd_catalog_prt, track_no);

  local_key_datum.dptr = key_to_del;
  local_key_datum.dsize = sizeof(key_to_del);

  result = dbm_delete(cdt_dbm_ptr, local_key_datum);

  if (result == 0)
    return 1;

  return 0;
}

cdc_entry search_cdc_entry(const char *cd_catalog_prt, int *first_call_ptr) {
  static int local_first_call = 1;
  cdc_entry entry_to_return;
  datum local_data_datum;
  static datum local_key_datum;

  memset(&entry_to_return, '\0', sizeof(entry_to_return));

  if (!cdc_dbm_ptr || !cdt_dbm_ptr) return entry_to_return;
  if (!cd_catalog_prt || !first_call_ptr) return entry_to_return;
  if (strlen(cd_catalog_prt) >= CAT_CAT_LEN) return entry_to_return;

  if (local_first_call) {
    local_first_call = 0;
    *first_call_ptr = 1;
  }

  if (*first_call_ptr) {
    *first_call_ptr = 0;
    local_key_datum = dbm_firstkey(cdc_dbm_ptr);
  } else {
    local_key_datum = dbm_nextkey(cdc_dbm_ptr);
  }

  do {
    if (local_key_datum.dptr != NULL) {
      local_data_datum = dbm_fetch(cdc_dbm_ptr, local_key_datum);
      if (local_data_datum.dptr) {
        memcpy(&entry_to_return, local_data_datum.dptr,
            local_data_datum.dsize);
        if (!strstr(entry_to_return.catalog, cd_catalog_prt)) {
          memset(&entry_to_return, '\0', sizeof(entry_to_return));
          local_key_datum = dbm_nextkey(cdc_dbm_ptr);
        }
      }
    }
  } while (local_key_datum.dptr && 
      local_data_datum.dptr &&
      (entry_to_return.catalog[0] == '\0'));


  return entry_to_return;
}
