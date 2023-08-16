#pragma once

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
#define CHAT_ID 123456 /* DEFAULT Chat id (int)*/
#define SYSTEM_NAME "YOUR USERNAME (on the linux system)"

#define API_TOKEN "YOUR_TOKEN"
//50 characters or more is recommended. You will most likely never need this. It is only for if Telegram goes down for any reason.
#define RECOVERY_PASSWORD "YOUR_RECOVERY_PASSWORD"


//If there is a user that is not whitelisted then you can send all un-whitelisted requests to the admin specified with 'CHAT_ID'
#define DEFAULT_AUTH_USER


#define USER_GROUPS_SIZE (1) //This is the number of whitelisted users (ass seen below)
const user_group_t USER_GROUPS[USER_GROUPS_SIZE] = {
	{CHAT_ID, SYSTEM_NAME}, //Format is {user_id (int), user_name (string)},
};


///////////////////