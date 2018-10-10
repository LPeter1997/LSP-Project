/**
 * rpc.hpp
 *
 * @author Peter Lenkefi
 * @date 2018-10-10
 * @description Provides a high-level interface to parse, construct and
 * interpret RPC messages.
 */

#ifndef LSP_RPC_HPP
#define LSP_RPC_HPP

#include <iostream>
#include "common.hpp"

namespace lsp {

/**
 * Response error.
 */
template <typename D>
struct response_err {
	response_err() = default;
	response_err(response_err const&) = default;
	response_err(response_err&&) = default;

	explicit response_err(i32 code, std::string&& msg, opt<D>&& error)
		: m_Code(code), m_Message(std::move(msg)), m_Data(std::move(error)) {
	}

	response_err& operator=(response_err const&) = default;
	response_err& operator=(response_err&&) = default;

	// Getters and setters
	i32& code() { return m_Code; }
	std::string& message() { return m_Message; }
	opt<D>& data() { return m_Data; }

	i32 const& code() const { return m_Code; }
	std::string const& message() const { return m_Message; }
	opt<D> const& data() const { return m_Data; }

	// Important operations
	json to_json() const {
		json res = {
			{ "code", code() },
			{ "message", message() },
		};
		if (auto const& data_field = data(); data_field) {
			res["data"] = *data_field;
		}
		return res;
	}

private:
	i32 m_Code;
	std::string m_Message;
	opt<D> m_Data;
};

namespace err_codes {
	// Defined by JSON-RPC
	inline constexpr i32 parse_error = -32700;
	inline constexpr i32 invalid_request = -32600;
	inline constexpr i32 method_not_found = -32601;
	inline constexpr i32 invalid_params = -32602;
	inline constexpr i32 internal_error = -32603;
	inline constexpr i32 server_error_start = -32099;
	inline constexpr i32 server_error_end = -32000;
	inline constexpr i32 server_not_initialized = -32002;
	inline constexpr i32 unknown_error_code = -32001;

	// Defined by LSP
	inline constexpr i32 request_cancelled = -32800;
}

/**
 * Response message.
 */
struct response_msg {
	response_msg() = default;
	response_msg(response_msg const&) = default;
	response_msg(response_msg&&) = default;

	explicit response_msg(json&& id, opt<json>&& result, opt<response_err<json>>&& error)
		: m_ID(std::move(id)),
		m_Result(std::move(result)), m_Error(std::move(error)) {
	}

	response_msg& operator=(response_msg const&) = default;
	response_msg& operator=(response_msg&&) = default;

	// Getters and setters
	json& id() { return m_ID; }
	opt<json>& result() { return m_Result; }
	opt<response_err<json>>& error() { return m_Error; }

	json const& id() const { return m_ID; }
	opt<json> const& result() const { return m_Result; }
	opt<response_err<json>> const& error() const { return m_Error; }

	// Important operations
	json to_json() const;

private:
	json m_ID;
	opt<json> m_Result;
	opt<response_err<json>> m_Error;
};

/**
 * Request message.
 */
struct request_msg {
	request_msg() = default;
	request_msg(request_msg const&) = default;
	request_msg(request_msg&&) = default;

	explicit request_msg(json&& id, std::string&& method, opt<json>&& params)
		: m_ID(std::move(id)),
		m_Method(std::move(method)), m_Params(std::move(params)) {
	}

	request_msg& operator=(request_msg const&) = default;
	request_msg& operator=(request_msg&&) = default;

	// Getters and setters
	json& id() { return m_ID; }
	std::string& method() { return m_Method; }
	opt<json>& params() { return m_Params; }

	json const& id() const { return m_ID; }
	std::string const& method() const { return m_Method; }
	opt<json> const& params() const { return m_Params; }

	// Important operations
	json to_json() const;
	response_msg response() const;

private:
	json m_ID;
	std::string m_Method;
	opt<json> m_Params;
};

/**
 * Notification message.
 */
struct notification_msg {
	notification_msg() = default;
	notification_msg(notification_msg const&) = default;
	notification_msg(notification_msg&&) = default;

	explicit notification_msg(std::string&& method, opt<json>&& params)
		: m_Method(std::move(method)), m_Params(std::move(params)) {
	}

	notification_msg& operator=(notification_msg const&) = default;
	notification_msg& operator=(notification_msg&&) = default;

	// Getters and setters
	std::string& method() { return m_Method; }
	opt<json>& params() { return m_Params; }

	std::string const& method() const { return m_Method; }
	opt<json> const& params() const { return m_Params; }

	// Important operations
	json to_json() const;

private:
	std::string m_Method;
	opt<json> m_Params;
};

/**
 * The base message class.
 */
struct msg {
	enum kind {
		request,
		response,
		notification,
	};

	template <typename T>
	msg(T&& val)
		: m_Data(std::forward<T>(val)) {
	}

	kind type() const {
		switch (m_Data.index()) {
		case 0: return request;
		case 1: return response;
		case 2: return notification;
		default: lsp_unreachable;
		}
	}

	bool is_request() const { return type() == request; }
	bool is_response() const { return type() == response; }
	bool is_notification() const { return type() == notification; }

	request_msg& as_request() { return std::get<request_msg>(m_Data); }
	request_msg const& as_request() const { return std::get<request_msg>(m_Data); }

	response_msg& as_response() { return std::get<response_msg>(m_Data); }
	response_msg const& as_response() const { return std::get<response_msg>(m_Data); }

	notification_msg& as_notification() { return std::get<notification_msg>(m_Data); }
	notification_msg const& as_notification() const { return std::get<notification_msg>(m_Data); }

	// Important operations
	json to_json() const;

private:
	sum<request_msg, response_msg, notification_msg> m_Data;
};

// XXX(LPeter1997): Some kind of error interface for the message reader?

/**
 * A message reader that reads RPC messages from a given source.
 */
struct msg_reader {
	explicit msg_reader(std::istream& stream)
		: m_Stream(&stream) {
	}

	msg next();

private:
	std::istream* m_Stream;
};

} /* namespace lsp */

#endif /* LSP_RPC_HPP */
