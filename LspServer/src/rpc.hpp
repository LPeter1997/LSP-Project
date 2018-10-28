/**
 * rpc.hpp
 *
 * @author Peter Lenkefi
 * @date 2018-10-23
 * @description Here are the minimal JSON-RPC utilities that the LSP needs.
 */

#ifndef RPC_HPP
#define RPC_HPP

#include <string>
#include <optional>
#include <variant>
#include "common.hpp"

namespace lsp {
namespace rpc {

struct response;

#define ctors(x) 					\
x(x const&) = default; 				\
x(x&&) = default; 					\
x& operator=(x const&) = default; 	\
x& operator=(x&&) = default

/**
 * Error codes that are either specified by RPC or the LSP protocol.
 */
namespace error_code {
	// JSON-RPC
	inline constexpr i32 parse_error = -32700;
	inline constexpr i32 invalid_request = -32600;
	inline constexpr i32 method_not_found = -32601;
	inline constexpr i32 invalid_params = -32602;
	inline constexpr i32 internal_error = -32603;
	inline constexpr i32 server_error_start = -32099;
	inline constexpr i32 server_error_end = -32000;
	inline constexpr i32 server_not_initialized = -32002;
	inline constexpr i32 unknown_error_code = -32001;

	// LSP
	inline constexpr i32 request_cancelled = -32800;
} /* namespace error_code */

/**
 * Part of the Response message if an error is signaled.
 */
template <typename D>
struct response_error {
	ctors(response_error);

	template <typename TMsg, typename TData = std::optional<D>>
	explicit response_error(i32 code, TMsg&& msg, TData&& data = std::nullopt)
		: m_Code(code),
		m_Message(std::forward<TMsg>(msg)), m_Data(std::forward<TData>(data)) {}

	auto const& code() const { return m_Code; }
	auto const& message() const { return m_Message; }
	auto const& data() const { return m_Data; }

	json to_json() const {
		json res = {
			{ "code", code() },
			{ "message", message() },
		};
		if (auto const& d = data()) {
			res["data"] = *d;
		}
		return res;
	}

private:
	i32 m_Code;
	std::string m_Message;
	std::optional<D> m_Data;
};

/**
 * The Response (Request reply) message type.
 */
struct response {
	ctors(response);

	template <typename TID, typename TRes, typename TErr = std::optional<response_error<json>>>
	explicit response(TID&& id,
		TRes&& result = (json)nullptr, TErr&& error = std::nullopt)
		: m_ID(std::forward<TID>(id)),
		m_Result(std::forward<TRes>(result)),
		m_Error(std::forward<TErr>(error)) {}

	auto const& result() const { return m_Result; }
	auto const& error() const { return m_Error; }

	bool has_error() const { return m_Error.has_value(); }

	json to_json() const;

private:
	json m_ID;
	json m_Result;
	std::optional<response_error<json>> m_Error;
};

/**
 * The Request message type.
 */
struct request {
	ctors(request);

	template <typename TID, typename TMethod, typename TParams>
	explicit request(TID&& id, TMethod&& method, TParams&& params = {})
		: m_ID(std::forward<TID>(id)),
		m_Method(std::forward<TMethod>(method)),
		m_Params(std::forward<TParams>(params)) {}

	auto const& method() const { return m_Method; }
	auto const& params() const { return m_Params; }

	json to_json() const;

	template <typename... Ts>
	response reply(Ts&&... args) const {
		return response(m_ID, std::forward<Ts>(args)...);
	}

private:
	json m_ID;
	std::string m_Method;
	json m_Params;
};

/**
 * The Notification message type.
 */
struct notification {
	ctors(notification);

	template <typename TMethod, typename TParams>
	explicit notification(TMethod&& method, TParams&& params = {})
		: m_Method(std::forward<TMethod>(method)),
		m_Params(std::forward<TParams>(params)) {}

	auto const& method() const { return m_Method; }
	auto const& params() const { return m_Params; }

	json to_json() const;

private:
	std::string m_Method;
	json m_Params;
};

/**
 * The generic RPC message that's either a Request, Response or Notification.
 */
struct message {
	ctors(message);

	template <typename TSub>
	message(TSub&& val)
		: m_Data(std::forward<TSub>(val)) {}

	static message parse(char const* msg);

	bool is_request() const {
		return std::holds_alternative<request>(m_Data);
	}

	bool is_response() const {
		return std::holds_alternative<response>(m_Data);
	}

	bool is_notification() const {
		return std::holds_alternative<notification>(m_Data);
	}

	auto const& as_request() const {
		lsp_assert(is_request());
		return std::get<request>(m_Data);
	}

	auto const& as_response() const {
		lsp_assert(is_response());
		return std::get<response>(m_Data);
	}

	auto const& as_notification() const {
		lsp_assert(is_notification());
		return std::get<notification>(m_Data);
	}

	json to_json() const;

private:
	std::variant<request, response, notification> m_Data;
};

#undef ctors

} /* namespace rpc */
} /* namespace lsp */

#endif /* RPC_HPP */
