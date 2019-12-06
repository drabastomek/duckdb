//===----------------------------------------------------------------------===//
//                         DuckDB
//
// main/appender.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/types/data_chunk.hpp"
#include "duckdb/main/table_description.hpp"

namespace duckdb {

class ClientContext;
class DuckDB;
class TableCatalogEntry;
class Connection;

//! The Appender class can be used to append elements to a table.
class Appender {
	//! A reference to a database connection that created this appender
	Connection &con;
	//! The table description (including column names)
	unique_ptr<TableDescription> description;
	//! Internal chunk used for appends
	DataChunk chunk;
	//! The current column to append to
	index_t column = 0;
	//! Message explaining why the Appender is invalidated (if any)
	string invalidated_msg;
public:
	Appender(Connection &con, string schema_name, string table_name);
	Appender(Connection &con, string table_name);
	~Appender();

	//! Begins a new row append, after calling this the other AppendX() functions
	//! should be called the correct amount of times. After that,
	//! EndRow() should be called.
	void BeginRow();
	//! Finishes appending the current row.
	void EndRow();

	// Append functions
	template<class T>
	void Append(T value) {
		throw Exception("Undefined type for Appender::Append!");
	}

	// prepared statements
	template <typename... Args> void AppendRow(Args... args) {
		BeginRow();
		AppendRowRecursive(args...);
	}

	//! Commit the changes made by the appender.
	void Flush();
	//! Flush the changes made by the appender and close it. The appender cannot be used after this point
	void Close();

	index_t CurrentColumn() {
		return column;
	}

	void Invalidate(string msg);

private:
	bool CheckAppend(TypeId type);
	void CheckInvalidated();

	void AppendRowRecursive() {
		EndRow();
	}

	template <typename T, typename... Args>
	void AppendRowRecursive(T value, Args... args) {
		Append<T>(value);
		AppendRowRecursive(args...);
	}

	void AppendValue(Value value);
};

template <> void Appender::Append(bool value);
template <> void Appender::Append(int8_t value);
template <> void Appender::Append(int16_t value);
template <> void Appender::Append(int32_t value);
template <> void Appender::Append(int64_t value);
template <> void Appender::Append(double value);
template <> void Appender::Append(const char* value);
template <> void Appender::Append(Value value);
template <> void Appender::Append(std::nullptr_t value);

} // namespace duckdb