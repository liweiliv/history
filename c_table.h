/*
 * c_table.h
 *
 *  Created on: 2016年12月8日
 *      Author: liwei
 */

#ifndef LIB_CALCULATE_ENGINE_C_TABLE_H_
#define LIB_CALCULATE_ENGINE_C_TABLE_H_
#include "c_block.h"
#include "db_chain.h"
#include "BR.h"
#include "BinlogRecord.h"
struct _c_database;

typedef struct _c_table{
    char *tbname;
    struct c_database *db;
    char index_file[256];
    char head_file[256];
    c_table_block* current_block;
    chain_head block_cache_head;
}c_table;




#endif /* LIB_CALCULATE_ENGINE_C_TABLE_H_ */
