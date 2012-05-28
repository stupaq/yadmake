/*
 * This file is part of the SSH Library
 *
 * Copyright (c) 2010 by Aris Adamantiadis
 *
 * The SSH Library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * The SSH Library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the SSH Library; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef LIBSSHPP_HPP_
#define LIBSSHPP_HPP_

/**
 * @defgroup ssh_cpp The libssh C++ wrapper
 *
 * The C++ bindings for libssh are completely embedded in a single .hpp file, and
 * this for two reasons:
 * - C++ is hard to keep binary compatible, C is easy. We try to keep libssh C version
 *   as much as possible binary compatible between releases, while this would be hard for
 *   C++. If you compile your program with these headers, you will only link to the C version
 *   of libssh which will be kept ABI compatible. No need to recompile your C++ program
 *   each time a new binary-compatible version of libssh is out
 * - Most of the functions in this file are really short and are probably worth the "inline"
 *   linking mode, which the compiler can decide to do in some case. There would be nearly no
 *   performance penalty of using the wrapper rather than native calls.
 *
 * Please visit the documentation of ssh::Session and ssh::Channel
 * @see ssh::Session
 * @see ssh::Channel
 *
 * If you wish not to use C++ exceptions, please define SSH_NO_CPP_EXCEPTIONS:
 * @code
 * #define SSH_NO_CPP_EXCEPTIONS
 * #include <libssh/libsshpp.hpp>
 * @endcode
 * All functions will then return SSH_ERROR in case of error.
 * @{
 */

/* do not use deprecated functions */
#define LIBSSH_LEGACY_0_4

#include <libssh/libssh.h>
#include <libssh/server.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

/* WTF?! TODO */
#include <string>

namespace ssh {

	class Channel;
	/** Some people do not like C++ exceptions. With this define, we give
	 * the choice to use or not exceptions.
	 * @brief if defined, disable C++ exceptions for libssh c++ wrapper
	 */
#ifndef SSH_NO_CPP_EXCEPTIONS

	/** @brief This class describes a SSH Exception object. This object can be thrown
	 * by several SSH functions that interact with the network, and may fail because of
	 * socket, protocol or memory errors.
	 */
	class SshException{
		public:
			SshException(ssh_session csession);
			SshException(const SshException &e);
			/** @brief returns the Error code
			 * @returns SSH_FATAL Fatal error happened (not recoverable)
			 * @returns SSH_REQUEST_DENIED Request was denied by remote host
			 * @see ssh_get_error_code
			 */
			int getCode();
			/** @brief returns the error message of the last exception
			 * @returns pointer to a c string containing the description of error
			 * @see ssh_get_error
			 */
			std::string getError();
		private:
			int code;
			std::string description;
	};

	/** @internal
	 * @brief Macro to throw exception if there was an error
	 */
#define ssh_throw(x) if((x)==SSH_ERROR) throw SshException(getCSession())
#define ssh_throw_null(CSession,x) if((x)==NULL) throw SshException(CSession)
#define void_throwable void
#define return_throwable return

#else

	/* No exception at all. All functions will return an error code instead
	 * of an exception
	 */
#define ssh_throw(x) if((x)==SSH_ERROR) return SSH_ERROR
#define ssh_throw_null(CSession,x) if((x)==NULL) return NULL
#define void_throwable int
#define return_throwable return SSH_OK
#endif

	/**
	 * The ssh::Session class contains the state of a SSH connection.
	 */
	class Session {
		friend class Channel;
		public:
		Session();
		~Session();
		/** @brief sets an SSH session options
		 * @param type Type of option
		 * @param option cstring containing the value of option
		 * @throws SshException on error
		 * @see ssh_options_set
		 */
		void_throwable setOption(enum ssh_options_e type, const char *option);
		/** @brief sets an SSH session options
		 * @param type Type of option
		 * @param option long integer containing the value of option
		 * @throws SshException on error
		 * @see ssh_options_set
		 */
		void_throwable setOption(enum ssh_options_e type, long int option);
		/** @brief sets an SSH session options
		 * @param type Type of option
		 * @param option void pointer containing the value of option
		 * @throws SshException on error
		 * @see ssh_options_set
		 */
		void_throwable setOption(enum ssh_options_e type, void *option);
		/** @brief connects to the remote host
		 * @throws SshException on error
		 * @see ssh_connect
		 */
		void_throwable connect();
		/** @brief Authenticates automatically using public key
		 * @throws SshException on error
		 * @returns SSH_AUTH_SUCCESS, SSH_AUTH_PARTIAL, SSH_AUTH_DENIED
		 * @see ssh_userauth_autopubkey
		 */
		int userauthAutopubkey(void);
		/** @brief Authenticates using the "none" method. Prefer using autopubkey if
		 * possible.
		 * @throws SshException on error
		 * @returns SSH_AUTH_SUCCESS, SSH_AUTH_PARTIAL, SSH_AUTH_DENIED
		 * @see ssh_userauth_none
		 * @see Session::userauthAutoPubkey
		 */
		int userauthNone();
		/** @brief Authenticates using the password method.
		 * @param[in] password password to use for authentication
		 * @throws SshException on error
		 * @returns SSH_AUTH_SUCCESS, SSH_AUTH_PARTIAL, SSH_AUTH_DENIED
		 * @see ssh_userauth_password
		 */
		int userauthPassword(const char *password);
		/** @brief Try to authenticate using the publickey method.
		 * @param[in] type public key type
		 * @param[in] pubkey public key to use for authentication
		 * @throws SshException on error
		 * @returns SSH_AUTH_SUCCESS if the pubkey is accepted,
		 * @returns SSH_AUTH_DENIED if the pubkey is denied
		 * @see ssh_userauth_offer_pubkey
		 */
		int userauthOfferPubkey(int type, ssh_string pubkey);
		/** @brief Authenticates using the publickey method.
		 * @param[in] pubkey public key to use for authentication
		 * @param[in] privkey private key to use for authentication
		 * @throws SshException on error
		 * @returns SSH_AUTH_SUCCESS, SSH_AUTH_PARTIAL, SSH_AUTH_DENIED
		 * @see ssh_userauth_pubkey
		 */
		int userauthPubkey(ssh_string pubkey, ssh_private_key privkey);
		int userauthPubkey(ssh_private_key privkey);
		int userauthPrivatekeyFile(const char *filename,
				const char *passphrase);
		/** @brief Returns the available authentication methods from the server
		 * @throws SshException on error
		 * @returns Bitfield of available methods.
		 * @see ssh_userauth_list
		 */
		int getAuthList();
		/** @brief Disconnects from the SSH server and closes connection
		 * @see ssh_disconnect
		 */
		void disconnect();
		/** @brief Returns the disconnect message from the server, if any
		 * @returns pointer to the message, or NULL. Do not attempt to free
		 * the pointer.
		 */
		const char *getDisconnectMessage();
		/** @internal
		 * @brief gets error message
		 */
		const char *getError();
		/** @internal
		 * @brief returns error code
		 */
		int getErrorCode();
		/** @brief returns the file descriptor used for the communication
		 * @returns the file descriptor
		 * @warning if a proxycommand is used, this function will only return
		 * one of the two file descriptors being used
		 * @see ssh_get_fd
		 */
		socket_t getSocket();
		/** @brief gets the Issue banner from the ssh server
		 * @returns the issue banner. This is generally a MOTD from server
		 * @see ssh_get_issue_banner
		 */
		std::string getIssueBanner();
		/** @brief returns the OpenSSH version (server) if possible
		 * @returns openssh version code
		 * @see ssh_get_openssh_version
		 */
		int getOpensshVersion();
		/** @brief returns the version of the SSH protocol being used
		 * @returns the SSH protocol version
		 * @see ssh_get_version
		 */
		int getVersion();
		/** @brief verifies that the server is known
		 * @throws SshException on error
		 * @returns Integer value depending on the knowledge of the
		 * server key
		 * @see ssh_is_server_known
		 */
		int isServerKnown();
		void log(int priority, const char *format, ...);

		/** @brief copies options from a session to another
		 * @throws SshException on error
		 * @see ssh_options_copy
		 */
		void_throwable optionsCopy(const Session &source);
		/** @brief parses a configuration file for options
		 * @throws SshException on error
		 * @param[in] file configuration file name
		 * @see ssh_options_parse_config
		 */
		void_throwable optionsParseConfig(const char *file);
		/** @brief silently disconnect from remote host
		 * @see ssh_silent_disconnect
		 */
		void silentDisconnect();
		/** @brief Writes the known host file with current
		 * host key
		 * @throws SshException on error
		 * @see ssh_write_knownhost
		 */
		int writeKnownhost();

		/** @brief accept an incoming forward connection
		 * @param[in] timeout_ms timeout for waiting, in ms
		 * @returns new Channel pointer on the forward connection
		 * @returns NULL in case of error
		 * @warning you have to delete this pointer after use
		 * @see ssh_channel_forward_accept
		 * @see Session::listenForward
		 */
		Channel *acceptForward(int timeout_ms);
		/* acceptForward is implemented later in this file */

		void_throwable cancelForward(const char *address, int port);

		void_throwable listenForward(const char *address, int port,
				int &boundport);

		private:
		ssh_session c_session;
		ssh_session getCSession();
		/* No copy constructor, no = operator */
		Session(const Session &);
		Session& operator=(const Session &);
	};

	/** @brief the ssh::Channel class describes the state of an SSH
	 * channel.
	 * @see ssh_channel
	 */
	class Channel {
		friend class Session;
		public:
		Channel(Session &session);
		~Channel();

		/** @brief accept an incoming X11 connection
		 * @param[in] timeout_ms timeout for waiting, in ms
		 * @returns new Channel pointer on the X11 connection
		 * @returns NULL in case of error
		 * @warning you have to delete this pointer after use
		 * @see ssh_channel_accept_x11
		 * @see Channel::requestX11
		 */
		Channel *acceptX11(int timeout_ms);
		/** @brief change the size of a pseudoterminal
		 * @param[in] cols number of columns
		 * @param[in] rows number of rows
		 * @throws SshException on error
		 * @see ssh_channel_change_pty_size
		 */
		void_throwable changePtySize(int cols, int rows);

		/** @brief closes a channel
		 * @throws SshException on error
		 * @see ssh_channel_close
		 */
		void_throwable close();

		int getExitStatus();

		Session &getSession();
		/** @brief returns true if channel is in closed state
		 * @see ssh_channel_is_closed
		 */
		bool isClosed();
		/** @brief returns true if channel is in EOF state
		 * @see ssh_channel_is_eof
		 */
		bool isEof();
		/** @brief returns true if channel is in open state
		 * @see ssh_channel_is_open
		 */
		bool isOpen();

		int openForward(const char *remotehost, int remoteport,
				const char *sourcehost=NULL, int localport=0);

		/* TODO: completely remove this ? */
		void_throwable openSession();

		int poll(bool is_stderr=false);

		int read(void *dest, size_t count, bool is_stderr=false);

		int readNonblocking(void *dest, size_t count, bool is_stderr=false);

		void_throwable requestEnv(const char *name, const char *value);

		void_throwable requestExec(const char *cmd);

		void_throwable requestPty(const char *term=NULL, int cols=0, int rows=0);

		void_throwable requestShell();

		void_throwable requestSendSignal(const char *signum);

		void_throwable requestSubsystem(const char *subsystem);

		int requestX11(bool single_connection,
				const char *protocol, const char *cookie, int screen_number);

		void_throwable sendEof();

		/** @brief Writes on a channel
		 * @param data data to write.
		 * @param len number of bytes to write.
		 * @param is_stderr write should be done on the stderr channel (server only)
		 * @returns number of bytes written
		 * @throws SshException in case of error
		 * @see channel_write
		 * @see channel_write_stderr
		 */
		int write(const void *data, size_t len, bool is_stderr=false);
		private:
		ssh_session getCSession();

		Channel (Session &session, ssh_channel c_channel);

		Session *session;
		ssh_channel channel;
		/* No copy and no = operator */
		Channel(const Channel &);
		Channel &operator=(const Channel &);
	};


	/* This code cannot be put inline due to references to Channel */
	// Channel *Session::acceptForward(int timeout_ms);

} // namespace ssh

/** @} */
#endif /* LIBSSHPP_HPP_ */
