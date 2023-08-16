echo "Install script v1.0"
echo "Please open your telegram app"
echo "(press enter to continue)"
read junk
clear
echo "Open a DM with https://t.me/BotFather  (click or copy and paste the link)"
echo "(press enter to continue)"
read junk
clear
echo "Do \"/start\""
echo "(press enter to continue)"
read junk
clear
echo "Do \"/newbot\""
echo "(press enter to continue)"
read junk
clear


echo "Please enter your API token: "
read token

if [ -z "$token" ]; then
	echo "ERROR: You typed in an empty token! Please re-run the program"
	exit 1
fi

#dont ask how this works. This took me hours
chat_id=$(curl "https://api.telegram.org/bot$token/getUpdates?limit=1&offset=-1" | sed -n -e 's/^.*chat\"\:{\"id\"\://p' | cut -d',' -f1)





echo "Enable recovery password? (true/false)"
read enable_rec

if [ $enable_rec = "true" ]
then
	echo "Enter a recovery password"
	read pass

	if [ -z "$pass" ]; then
		echo "ERROR: Password was empty! Please re-run script"
		exit 1
	fi
fi
echo "How long to wait before checking for more messages? (recomended is 3 seconds)"
read delay

if [ -z "$delay" ]; then
	echo "ERROR: Delay was null! Please re-run script!"
	exit 1
fi

echo "If a user is not reconized, should ...\n\t- An admin still get a message for accepting or denying the request? (true)\n\t- Deny any users who are not whitelisted in settings.h (false)\nDEFAULT IS 'true'"
read enable_default_chat

if [ -z "$enable_default_chat" ]; then
	echo "No info was given. Defaulting to 'true'"
	enable_default_chat="true"
fi




echo "What directory would you like to install the module to? (Default is /usr/local/lib/pam_addons/)"
read exp_dir

if [ -z "$exp_dir" ]; then
	exp_dir="/usr/local/lib/pam_addons"
fi





echo "Generating settings.h ..."
echo "" > settings.h
echo "#pragma once\n" >> settings.h
echo "#define CHAT_ID $chat_id\n" >> settings.h
echo "#define API_TOKEN \"$token\"" >> settings.h
echo "#define UPDATE_DELAY $delay" >> settings.h

if [ $enable_rec = "true" ]
then
	echo "#define RECOVERY_PASSWORD \"$pass\"" >> settings.h
else
	echo "#define PAM_TELEGRAM_NO_RECOVERY" >> settings.h
fi
if [ $enable_default_chat = "true" ]
then
	echo "#define DEFAULT_AUTH_USER" >> settings.h
fi

echo "#define USER_GROUPS_SIZE (1)" >> settings.h
echo "const user_group_t USER_GROUPS[USER_GROUPS_SIZE] = {" >> settings.h
echo "\t{$chat_id, \"$(whoami)\"}," >> settings.h
echo "};" >> settings.h

clear
echo "Done generating settings!\n"
echo "Your default editor will now open to ensure that the settings were correct in 10 seconds ..."
sleep 10
$EDITOR settings.h

clear
echo "Please enter sudo for making the PAM directory"
sudo mkdir -p "$exp_dir"

clear
echo "Installing dependancies ..."
make depend
#remove duplicates
FILE=json.hpp
if [ -f "$FILE" ]; then
    rm json.hpp.1
fi

clear
echo "Compiling program ...\n"
make compile

clear
echo "Copying program to directory ..."
sudo cp ./build/pam_telegram.so "$exp_dir"

clear
echo "Done! Follow the README.md for further instructions on how to configure PAM"