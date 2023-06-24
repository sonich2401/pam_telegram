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
#define CHAT_ID "YOUR_CHAT"

#define API_TOKEN "YOUR_TOKEN"
//50 characters or more is recommended. You will most likely never need this. It is only for if Telegram goes down for any reason.
#define RECOVERY_PASSWORD "YOUR_RECOVERY_PASSWORD"
///////////////////
