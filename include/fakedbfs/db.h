/*
 * Copyright (c) 2005, Dan Ponte
 *
 * db.h - database stuff
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/* $Amigan: fakedbfs/include/fakedbfs/db.h,v 1.7 2006/04/19 19:58:22 dcp1990 Exp $ */
/**
 * @file db.h
 * @brief Database driver stuff
 */
#ifndef HAVE_FDBFS_DB_H
#define HAVE_FDBFS_DB_H
#include <fakedbfs/types.h>

/**
 * @brief Open the database and initialise sqlite.
 *
 * fdbfs_db_open() opens our database and sets certain sqlite paramaters.
 * @param f The instance of fakedbfs on which to operate.
 * @return Non-zero on success, zero on error.
 */
int fdbfs_db_open(fdbfs_t *f);

/**
 * @brief Close the database.
 *
 * fdbfs_db_close() closes the database.
 * @param f The instance of fakedbfs on which to operate.
 * @return Non-zero on success, zero on error.
 */
int fdbfs_db_close(fdbfs_t *f);

/**
 * @brief Check if a table exists in the database.
 *
 * fdbfs_db_table_exists() looks at the sqlite_master table to see if a table exists.
 * @param f The instance of fakedbfs on which to operate.
 * @param tname The tablename to check for.
 * @return Non-zero on existence, zero on error or no existence.
 */
int fdbfs_db_table_exists(fdbfs_t *f, const char *tname);

/**
 * @brief Check if a catalogue exists in the database.
 *
 * fdbfs_db_cat_exists() looks at the cat_list table to see if a catalogue exists.
 * @param f The instance of fakedbfs on which to operate.
 * @param cat The catalogue name to check for.
 * @return Non-zero on existence, zero on error or no existence.
 */
int fdbfs_db_cat_exists(fdbfs_t *f, const char *cat);

/**
 * @brief Create a table.
 *
 * fdbfs_db_create_table() creates a table of the specified name and specification.
 * @param f The instance of fakedbfs.
 * @param tname The name of the new table.
 * @param tspec The database-specific specification of the new table.
 * @return 0 on error, 1 on success.
 */
int fdbfs_db_create_table(fdbfs_t *f, const char *tname, const char *tspec);

/**
 * @brief Get the database-specific typename for an DataType enum.
 *
 * fdbfs_db_gettype() returns the database-specific type string for a specified DataType enum.
 * @param t The type to retrieve.
 * @return A string containing the database-specific type.
 */
const char* fdbfs_db_gettype(enum DataType t);

/**
 * @brief Add a column to a table.
 *
 * fdbfs_db_add_column adds a column to a table. This isn't exactly production-ready.
 * It requires that the version of SQLite support the "ALTER TABLE ADD COLUMN" command.
 * @param f The instance of fakedbfs.
 * @param tname The name of the table to operate on.
 * @param cname The name of the new column.
 * @param datatype The datatype of the column.
 * @return 0 on error, non-zero on success.
 */
int fdbfs_db_add_column(fdbfs_t *f, const char *tname, const char *cname, coltype_t datatype);

/**
 * @brief Delete a column from a table.
 *
 * fdbfs_db_del_column() deletes a column from a table if the database driver supports it. SQLite doesn't, so for now this does nothing.
 * @param f The instance of fakedbfs.
 * @param tname The table from which to remove the column.
 * @param cname The column to remove.
 * @return 0 on error, 1 otherwise.
 */
int fdbfs_db_del_column(fdbfs_t *f, const char *tname, const char *cname);

/**
 * @brief Add an entry to the enum_list table.
 *
 * fdbfs_db_add_to_enum_list_table() adds an entry to the enum list table.
 * @param f The instance of fakedbfs.
 * @param name The name of the enum.
 * @param tname The table that the enum is defined in.
 * @param specf The name of the filename the enum was originally defined in.
 * @return 0 on error, non-zero otherwise.
 */
int fdbfs_db_add_to_enum_list_table(fdbfs_t *f, const char *name, const char *tname, const char *specf);

/**
 * @brief Add an entry to the cat_list table.
 *
 * fdbfs_db_add_to_cat_list_table() adds an entry to the cat list table.
 * @param f The instance of fakedbfs.
 * @param name The name of the catalogue.
 * @param alias The friendly alias of the catalogue.
 * @param tablename The table that the catalogue is defined in.
 * @param fieldtable The field description table for this catalogue.
 * @param specf The name of the filename the catalogue was originally defined in.
 * @return 0 on error, non-zero otherwise.
 */
int fdbfs_db_add_to_cat_list_table(fdbfs_t *f, const char *name, const char *alias, const char *tablename,
		const char *fieldtable);

/**
 * @brief Add to the field description table for a catalogue.
 *
 * fdbfs_db_add_to_field_desc() adds an entry to the field description table for a catalogue.
 * @param f The instance of fakedbfs.
 * @param tablename The name of the field description table.
 * @param name The name of the field.
 * @param alias The friendly alias of the field.
 * @param type The datatype of the field.
 * @param typen The name of the enum, if applicable (NULL otherwise).
 * @return 0 on error, non-zero otherwise.
 */
int fdbfs_db_add_to_field_desc(fdbfs_t *f, const char *tablename, const char *name, const char *alias, enum DataType
		type, const char *typen);

/**
 * @brief Delete row from table in database.
 *
 * fdbfs_db_delete() deletes rows from a table based on certain conditions.
 * @param f The instance of fakedbfs.
 * @param from The table to operate on.
 * @param wherecol The column to check.
 * @param wherecmp The comparison operator. (==, etc)
 * @param whereval The value to check for.
 * @return 0 on error, non-zero otherwise.
 */
int fdbfs_db_delete(fdbfs_t *f, const char *from, const char *wherecol, const char *wherecmp, const char *whereval);

/**
 * @brief Drops a table from the database.
 *
 * fdbfs_db_drop_table() drops the named table from the database.
 * @param f The instance of fakedbfs.
 * @param tablename The name of the table to drop.
 * @return 0 on error, non-zero on success.
 */
int fdbfs_db_drop_table(fdbfs_t *f, const char *tablename);

/**
 * @brief Add an element to an enumeration description table.
 *
 * fdbfs_add_enum_elem() adds a row to an enum descripton table.
 * @param f The instance of fakedbfs.
 * @param tname The name of the enum description table.
 * @param name The name of the new element.
 * @param fmtname The friendly alias of the new element.
 * @param value The integer value of the new element.
 * @param dtype The datatype of the new element, if other.
 * @param subelements The subelements for this element.
 * @return 0 on error.
 */
int fdbfs_db_add_enum_elem(fdbfs_t *f, const char *tname, const char *name, const char *fmtname, unsigned int value,
		enum DataType dtype, const char *subelements);

/**
 * @brief Bind a field to a statement.
 *
 * fdbfs_db_bind_field() binds data to an SQLite statement. The field counter is increased on each call.
 * @param f The instance of fakedbfs.
 * @param[in,out] count A pointer to an integer specifying the index of the current field. This will be increased by one.
 * @param type The datatype of the field to bind.
 * @param value A pointer to the data to bind; the type of data is derived from the type parameter.
 * @param len For BLOB data, the length of data to bind. strlen() is used on strings, so this is not necessary for strings.
 * @param stmt The sqlite statement to operate on.
 * @return 0 on error.
 */
int fdbfs_db_bind_field(fdbfs_t *f, int *count, enum DataType type, void *value, size_t len, sqlite3_stmt *stmt);

/**
 * @brief Get the lastupdate time from a catalogue.
 *
 * fdbfs_db_get_lastupdate() gets the lastupdate time for a certain file in a catalogue.
 * @param f The instance of fakedbfs.
 * @param cat The catalogue to operate on.
 * @param filename The filename to query.
 * @retval -2 No match for this file.
 * @retval -1 Error.
 * @return The time of last updating on the specified file.
 */
int fdbfs_db_get_lastupdate(fdbfs_t *f, const char *cat, const char *filename);

/**
 * @brief Remove catalogue from database.
 *
 * @param f Instance of fakedbfs.
 * @param catname Catalogue name to remove.
 * @return 0 on error.
 */
int fdbfs_db_rm_catalogue(fdbfs_t *f, const const char *catname);

/**
 * @brief Adds a MIB to the configuration table.
 *
 * This is used by the configuration subsystem.
 * @param f The instance of fakedbfs.
 * @param mib The mib to add.
 * @param type The datatype of this MIB.
 * @param data The value of this MIB.
 * @return 0 on error, non-zero otherwise.
 */
int fdbfs_db_mib_add(fdbfs_t *f, const char *mib, enum DataType type, union Data data);

/**
 * @brief Updates a MIB in the configuration table.
 *
 * This is used by the configuration subsystem.
 * @param f The instance of fakedbfs.
 * @param mib The mib to update.
 * @param type The datatype of this MIB.
 * @param data The value of this MIB.
 * @return 0 on error, non-zero otherwise.
 */
int fdbfs_db_mib_update(fdbfs_t *f, const char *mib, enum DataType type, union Data data);

/**
 * @brief Initialise the database if necessary.
 * 
 * @param f Instance of fakedbfs to operate on.
 * @return 0 on error.
 */
int fdbfs_db_start(fdbfs_t *f);

/**
 * @brief Add entry to catalogue field definition list table.
 *
 * @param f Instance of fakedbfs to operate on.
 * @param name Name of new CFD.
 * @param alias Alias of new CFD.
 * @param tablename Table where this CFD resides.
 * @param specfile Specfile in which this CFD originated.
 * @return 0 on error.
 */
int fdbfs_db_add_to_cfd_list_table(fdbfs_t *f, const char *name, const char *alias, const char *tablename, const char *specfile);

/**
 * @brief Update refcount of CFD table entry.
 *
 * @param f Instance of fakedbfs to operate on.
 * @param name Name of CFD.
 * @param add True if adding, false if subtracting.
 * @param val Value to add to refcount.
 * @return 0 on error.
 */
int fdbfs_db_cfd_update_refcount(fdbfs_t *f, const char *name, int add, unsigned int val);

/**
 * @brief Get CFD name of table. (without cfd_ prefix)
 *
 * @param f Instance of fakedbfs.
 * @param catname Name of catalogue.
 * @param[out] tcfd Pointer to char* that will be set to cfdname (must be freed)
 * @return 0 on error, 1 otherwise.
 */
int fdbfs_db_cat_getcfdname(fdbfs_t *f, const char *catname, char **tcfd);

#endif
