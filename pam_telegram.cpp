#include <iostream>
#include <string>
#include <curl/curl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <security/pam_modules.h>
#include <security/pam_ext.h>
#include <string.h>
#include <sys/wait.h>
#include "json.hpp"



//SETTINGS

//Dont allow a recovery password to be used. This is NOT RECOMENDED. You must know what you are doing to use this option
//If you do use this option, please edit your /etc/pam.d/sshd config to skip this PAM module if a ssh key is being used or maybe
//if you are logging in as an admin or make a alternative login method.
//#define PAM_TELEGRAM_NO_RECOVERY

//The amount of seconds before checking for another message
#define UPDATE_DELAY 3


/*
Set this to the user you would like messages to be sent to
You can get this number by sending a message to the bot and then using this HTTP request
POST https://api.telegram.org/bot<API_TOKEN>/getUpdates?limit=1&offset=-1

You should find the chat_id inside the json response
*/
#define CHAT_ID "YOUR_CHAT"

#define API_TOKEN "YOUR_TOKEN"
//50 characters or more is recommended. You will most likely never need this. It is only for if Telegram goes down for any reason.
#define RECOVERY_PASSWORD "YOUR_RECOVERY_PASSWORD"
///////////////////




using json = nlohmann::json;
static std::string response_string;
static std::string header_string;


enum{
	TELE_ERR,
	TELE_ALLOW,
	TELE_DENY,
	TELE_UPDATE
};

static char* username = NULL;
static char* code = NULL;
static char result = TELE_UPDATE;

static json updates;

#define API_URL "https://api.telegram.org/bot"API_TOKEN

static CURL* curl = NULL;

static void parse_and_verify_response(){
	if(response_string == ""){ //No response but connected fine
		result = TELE_ERR;
		return;
	}
	updates = json::parse(response_string);
	if((unsigned char)updates["ok"] == 0){ //Unauthoritied
       result = TELE_ERR;
       return;
    }
}

static void generic_execute(){
	CURLcode res;
	response_string = "";
	/* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
  	/* Check for errors */
    if(res != CURLE_OK){
      	fprintf(stderr, "curl_easy_perform() failed: %s\n",
        curl_easy_strerror(res));

        result = TELE_ERR;
    }
    if(result == TELE_ERR){
    	return;
    }

    parse_and_verify_response();
}

//If the compiler does not support restrict then recover
#ifndef __restrict
	#define __restrict
#endif

static inline void generic_send_message(const char* __restrict options){
	curl_easy_setopt(curl, CURLOPT_URL, API_URL "/sendMessage");
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, options);
	generic_execute();
}



static void send_message(){
	char* options = NULL;
	asprintf(&options, "chat_id=" CHAT_ID "&text=User with the name \"%s\" is trying to sign in. /allow or /deny", username);
	generic_send_message(options);
	free(options);
}

static void send_code(){
	char* options = NULL;
	asprintf(&options, "chat_id=" CHAT_ID "&text=As you wish! Your login code is %s", code);
	generic_send_message(options);
	free(options);
}

static void send_deny(){
	char* options =  "chat_id=" CHAT_ID "&text=You shall not pass! (user was denied and disconencted)";
	generic_send_message(options);
}

static void get_message(){
	curl_easy_setopt(curl, CURLOPT_URL, API_URL"/getUpdates");
   	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "limit=1&offset=-1");
}


    

static size_t writeFunction(void *ptr, size_t size, size_t nmemb, std::string* data) {
	data->append((char*) ptr, size * nmemb);
	return size * nmemb;	        
}

static void wait_for_response(){
	unsigned long last_id = 0;

	get_message();

	while(1){
		generic_execute();

		//Initalize last message
		if(last_id == 0){
			last_id = (unsigned long)updates["result"][0]["update_id"];
			puts(std::string(updates["result"][0]["message"]["text"]).c_str());
			printf("ID %lu\n", last_id);
			continue;
		}

		//If we have not received a new message then keep looping
		if((unsigned long)updates["result"][0]["update_id"] == last_id){
			continue;
		}

		if(std::to_string((unsigned long)updates["result"][0]["message"]["from"]["id"]) != CHAT_ID){
			printf("UNAUTH %lu\n", (unsigned long)updates["result"][0]["message"]["from"]["id"]);
			continue;
		}
		
        //Check if the admin has allowed the user
		if(strcmp(std::string(updates["result"][0]["message"]["text"]).c_str(), "/allow") == 0){
			puts("ACCEPTED");
			result = TELE_ALLOW;
			send_code();
			break;
		}
		//Check if the admin has denied the user
		if(strcmp(std::string(updates["result"][0]["message"]["text"]).c_str(), "/deny") == 0){
			puts("DENY");
			result = TELE_DENY;
			send_deny();
			break;
		}
		sleep(UPDATE_DELAY); //Wait so we dont get rate limited

		//If there was an error then stop waiting for a message and report the error
		//to the user that is trying to log in
		if(result == TELE_ERR){
			return;
		}
	}
}






static void telegram()
{
  curl_global_init(CURL_GLOBAL_ALL);
 
  /* get a curl handle */
  curl = curl_easy_init();
  if(curl) {
  	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_string);
    
    send_message();
    if(result == TELE_ERR){
    	puts("RES FROM SEND MESSAGE ERR");
    	return;
    }
 
   	wait_for_response();

    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  curl_global_cleanup();
}




PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    int ret;

	result = TELE_UPDATE;
	srand(time(NULL));

	// Get the username
    ret = pam_get_user(pamh, (const char**)&username, NULL);
    if (ret != PAM_SUCCESS) {
        return ret;
    }

	code = NULL;
	asprintf(&code, "%X", rand());

	try{
		telegram();
	}
	//If there is any errors, require the recovery password (as seen later)
	catch(const std::exception& e){
		result = TELE_ERR;
	}
	
	//Deny the user access to the system if the telegram message was "/deny"
	if(result == TELE_DENY){
		return PAM_AUTH_ERR;
	}
    
	//Set up questionair
    int retval;
    const struct pam_conv *conv;
    struct pam_message msg;
    const struct pam_message *msgp;
    struct pam_response *resp;

 	retval = pam_get_item(pamh, PAM_CONV, (const void **)&conv);
    if (retval != PAM_SUCCESS || conv == NULL) {
        return PAM_AUTH_ERR;
    }

    msg.msg_style = PAM_PROMPT_ECHO_OFF;
    if(result == TELE_ALLOW){
    	msg.msg = "Check your phone for the auth code\nEnter code here: ";
	}
	#ifndef PAM_TELEGRAM_NO_RECOVERY
		//Let user know the bot is down but still allow login using recovery password
		else{
			msg.msg = "BOT IS DOWN! Use recovery password: ";
		}
	#else
		//We dont let users into the system if the bot is down
		else{
			msg.msg = "BOT IS DOWN! Please contact your admin to fix the issue!";
			return PAM_AUTH_ERR;
		}
	#endif
    msgp = &msg;
    resp = NULL;

    retval = conv->conv(1, &msgp, &resp, conv->appdata_ptr);
    if (retval != PAM_SUCCESS || resp == NULL) {
        return PAM_AUTH_ERR;
    }
	#ifndef PAM_TELEGRAM_NO_RECOVERY
    	char strcmp_res = (strcmp(resp->resp, RECOVERY_PASSWORD) == 0);
	#else
		char strcmp_res = 0;
	#endif

	if(result == TELE_ALLOW){
		strcmp_res |= (strcmp(resp->resp, code) == 0);
	}

    free(resp->resp);
    free(resp);

    if(strcmp_res) {
        return PAM_SUCCESS;
    }
    return PAM_AUTH_ERR;
}


PAM_EXTERN int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    return PAM_SUCCESS;
}