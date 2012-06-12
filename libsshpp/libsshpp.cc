#include "libsshpp.hpp"

using namespace ssh;

SshException::SshException(ssh_session csession){
	code=ssh_get_error_code(csession);
	description=std::string(ssh_get_error(csession));
}

SshException::SshException(const SshException &e){
	code=e.code;
	description=e.description;
}

int SshException::getCode(){
	return code;
}

std::string SshException::getError(){
	return description;
}

Session::Session(){
	c_session=ssh_new();
}

Session::~Session(){
	ssh_free(c_session);
	c_session=NULL;
}

void_throwable Session::setOption(enum ssh_options_e type, const char *option){
	ssh_throw(ssh_options_set(c_session,type,option));
	return_throwable;
}

void_throwable Session::setOption(enum ssh_options_e type, long int option){
	ssh_throw(ssh_options_set(c_session,type,&option));
	return_throwable;
}

void_throwable Session::setOption(enum ssh_options_e type, void *option){
	ssh_throw(ssh_options_set(c_session,type,option));
	return_throwable;
}

void_throwable Session::connect(){
	int ret=ssh_connect(c_session);
	ssh_throw(ret);
	return_throwable;
}

int Session::userauthAutopubkey(void){
	int ret=ssh_userauth_autopubkey(c_session,NULL);
	ssh_throw(ret);
	return ret;
}

int Session::userauthNone(){
	int ret=ssh_userauth_none(c_session,NULL);
	ssh_throw(ret);
	return ret;
}

int Session::userauthPassword(const char *password){
	int ret=ssh_userauth_password(c_session,NULL,password);
	ssh_throw(ret);
	return ret;
}

int Session::userauthOfferPubkey(int type, ssh_string pubkey){
	int ret=ssh_userauth_offer_pubkey(c_session,NULL,type,pubkey);
	ssh_throw(ret);
	return ret;
}

int Session::userauthPubkey(ssh_string pubkey, ssh_private_key privkey){
	int ret=ssh_userauth_pubkey(c_session,NULL,pubkey,privkey);
	ssh_throw(ret);
	return ret;
}

int Session::userauthPubkey(ssh_private_key privkey){
	int ret=ssh_userauth_pubkey(c_session,NULL,NULL,privkey);
	ssh_throw(ret);
	return ret;
}

/*int Session::userauthPrivatekeyFile(const char *filename,
  const char *passphrase);
  TODO
  */

int Session::getAuthList(){
	int ret=ssh_userauth_list(c_session, NULL);
	ssh_throw(ret);
	return ret;
}

void Session::disconnect(){
	ssh_disconnect(c_session);
}

const char *Session::getDisconnectMessage(){
	const char *msg=ssh_get_disconnect_message(c_session);
	return msg;
}

const char *Session::getError(){
	return ssh_get_error(c_session);
}

int Session::getErrorCode(){
	return ssh_get_error_code(c_session);
}

socket_t Session::getSocket(){
	return ssh_get_fd(c_session);
}

std::string Session::getIssueBanner(){
	char *banner=ssh_get_issue_banner(c_session);
	std::string ret= std::string(banner);
	::free(banner);
	return ret;
}

int Session::getOpensshVersion(){
	return ssh_get_openssh_version(c_session);
}

int Session::getVersion(){
	return ssh_get_version(c_session);
}

int Session::isServerKnown(){
	int ret=ssh_is_server_known(c_session);
	ssh_throw(ret);
	return ret;
}

void Session::log(int priority, const char *format, ...){
	char buffer[1024];
	va_list va;

	va_start(va, format);
	vsnprintf(buffer, sizeof(buffer), format, va);
	va_end(va);
	ssh_log(c_session,priority, "%s", buffer);
}

void_throwable Session::optionsCopy(const Session &source){
	ssh_throw(ssh_options_copy(source.c_session,&c_session));
	return_throwable;
}

void_throwable Session::optionsParseConfig(const char *file){
	ssh_throw(ssh_options_parse_config(c_session,file));
	return_throwable;
}

void Session::silentDisconnect(){
	ssh_silent_disconnect(c_session);
}

int Session::writeKnownhost(){
	int ret = ssh_write_knownhost(c_session);
	ssh_throw(ret);
	return ret;
}

void_throwable Session::cancelForward(const char *address, int port){
	int err=ssh_forward_cancel(c_session, address, port);
	ssh_throw(err);
	return_throwable;
}

void_throwable Session::listenForward(const char *address, int port,
		int &boundport){
	int err=ssh_forward_listen(c_session, address, port, &boundport);
	ssh_throw(err);
	return_throwable;
}

ssh_session Session::getCSession(){
	return c_session;
}

Channel::Channel(Session &session){
	channel=ssh_channel_new(session.getCSession());
	this->session=&session;
}

Channel::~Channel(){
	ssh_channel_free(channel);
	channel=NULL;
}

/** @brief accept an incoming X11 connection
 * @param[in] timeout_ms timeout for waiting, in ms
 * @returns new Channel pointer on the X11 connection
 * @returns NULL in case of error
 * @warning you have to delete this pointer after use
 * @see ssh_channel_accept_x11
 * @see Channel::requestX11
 */
Channel *Channel::acceptX11(int timeout_ms){
	ssh_channel x11chan = ssh_channel_accept_x11(channel,timeout_ms);
	ssh_throw_null(getCSession(),x11chan);
	Channel *newchan = new Channel(getSession(),x11chan);
	return newchan;
}

/** @brief change the size of a pseudoterminal
 * @param[in] cols number of columns
 * @param[in] rows number of rows
 * @throws SshException on error
 * @see ssh_channel_change_pty_size
 */
void_throwable Channel::changePtySize(int cols, int rows){
	int err=ssh_channel_change_pty_size(channel,cols,rows);
	ssh_throw(err);
	return_throwable;
}

/** @brief closes a channel
 * @throws SshException on error
 * @see ssh_channel_close
 */
void_throwable Channel::close(){
	ssh_throw(ssh_channel_close(channel));
	return_throwable;
}

int Channel::getExitStatus(){
	return ssh_channel_get_exit_status(channel);
}

Session &Channel::getSession(){
	return *session;
}

bool Channel::isClosed(){
	return ssh_channel_is_closed(channel) != 0;
}

bool Channel::isEof(){
	return ssh_channel_is_eof(channel) != 0;
}

bool Channel::isOpen(){
	return ssh_channel_is_open(channel) != 0;
}

int Channel::openForward(const char *remotehost, int remoteport,
		const char *sourcehost, int localport){ /* TODO default args given in hpp */
	int err=ssh_channel_open_forward(channel,remotehost,remoteport,
			sourcehost, localport);
	ssh_throw(err);
	return err;
}

void_throwable Channel::openSession(){
	int err=ssh_channel_open_session(channel);
	ssh_throw(err);
	return_throwable;
}

int Channel::poll(bool is_stderr){ /* TODO defeult.. */
	int err=ssh_channel_poll(channel,is_stderr);
	ssh_throw(err);
	return err;
}

int Channel::read(void *dest, size_t count, bool is_stderr){ /* TODO default .. */
	int err;
	/* handle int overflow */
	if(count > 0x7fffffff)
		count = 0x7fffffff;
	err=ssh_channel_read(channel,dest,count,is_stderr);
	ssh_throw(err);
	return err;
}

int Channel::readNonblocking(void *dest, size_t count, bool is_stderr){ /* TODO default .. */
	int err;
	/* handle int overflow */
	if(count > 0x7fffffff)
		count = 0x7fffffff;
	err=ssh_channel_read_nonblocking(channel,dest,count,is_stderr);
	ssh_throw(err);
	return err;
}

void_throwable Channel::requestEnv(const char *name, const char *value){
	int err=ssh_channel_request_env(channel,name,value);
	ssh_throw(err);
	return_throwable;
}

void_throwable Channel::requestExec(const char *cmd){
	int err=ssh_channel_request_exec(channel,cmd);
	ssh_throw(err);
	return_throwable;
}

void_throwable Channel::requestPty(const char *term, int cols, int rows){ /* TODO default.. */
	int err;
	if(term != NULL && cols != 0 && rows != 0)
		err=ssh_channel_request_pty_size(channel,term,cols,rows);
	else
		err=ssh_channel_request_pty(channel);
	ssh_throw(err);
	return_throwable;
}

void_throwable Channel::requestShell(){
	int err=ssh_channel_request_shell(channel);
	ssh_throw(err);
	return_throwable;
}

void_throwable Channel::requestSendSignal(const char *signum){
	int err=ssh_channel_request_send_signal(channel, signum);
	ssh_throw(err);
	return_throwable;
}

void_throwable Channel::requestSubsystem(const char *subsystem){
	int err=ssh_channel_request_subsystem(channel,subsystem);
	ssh_throw(err);
	return_throwable;
}

int Channel::requestX11(bool single_connection,
		const char *protocol, const char *cookie, int screen_number){
	int err=ssh_channel_request_x11(channel,single_connection,
			protocol, cookie, screen_number);
	ssh_throw(err);
	return err;
}

void_throwable Channel::sendEof(){
	int err=ssh_channel_send_eof(channel);
	ssh_throw(err);
	return_throwable;
}

int Channel::write(const void *data, size_t len, bool is_stderr){ /* TODO defaults */
	int ret;
	if(is_stderr){
		ret=ssh_channel_write_stderr(channel,data,len);
	} else {
		ret=ssh_channel_write(channel,data,len);
	}
	ssh_throw(ret);
	return ret;
}

ssh_session Channel::getCSession(){
	return session->getCSession();
}

Channel::Channel (Session &session, ssh_channel c_channel){
	this->channel=c_channel;
	this->session=&session;
}

/* This code cannot be put inline due to references to Channel */
Channel *Session::acceptForward(int timeout_ms){
	ssh_channel forward = ssh_forward_accept(c_session,
			timeout_ms);
	ssh_throw_null(c_session,forward);
	Channel *newchan = new Channel(*this,forward);
	return newchan;
}
