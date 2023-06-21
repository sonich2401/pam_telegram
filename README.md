# pam_telegram
2FA for linux login via the terminal such as ssh, ftp, and normal login

#Why did I make this?
I got a Oracle Cloud Compute server and found many people trying to login as root. Root login was disabled,
but I just didnt feel safe enabling username/password authentication. SSH keys solve this issue but if
you are at someone's laptop and you need to login then you are stuck because your SSH key is at home.

This solves that issue by enabling password login and only allows people who you allow manually.


#How it works
When someone successfully enters their password, this module is called. It sends a message to the owner
of the server via telegram. It reports what user is trying to sign in and gives the owner a choice
of accepting or denying the user. If the user replies to the telegram bot with "/allow" then a authentication
code is sent via telegram that is to be entered into the terminal as a password. The user is then given
access to the system.

#How to install
First install dependancies

```
sudo apt-get install libpam0g-dev libcurl4-openssl-dev
```
This is nessisary for interfacing with PAM and also interfacing with the internet


Inside of 'pam_telegram.cpp' there is a few settings you need to set.

The most important are the 'API_TOKEN' and the 'CHAT_ID'.
They are the required for interfacing with Telegram.
You can get a API_TOKEN by messaging the BotFather on Telegram and
running the '/newbot' command.

The CHAT_ID is a bit harder to get. Once you have your bot running, send it a message 
containing any text you want.

Then type this url into the 'curl' program in your Linux terminal or use it in your
favorite browser.

https://api.telegram.org/bot<API_TOKEN>/getUpdates?limit=1?offset=-1

You should see a bunch of text. Search for the words 'chat_id' and copy the value into
the 'CHAT_ID' macro in pam_telegram.cpp. MAKE SURE that it is contained in quotes so that it is a string.
If you do not do this then the compilation will error out.

I have included a script called "get_chat_id.sh" that should automate this for you.


If you want a recovery password just in case Telegram goes down, you can set that in the
'RECOVERY_PASSWORD' macro


Once you have set up all of this, go ahead and type in

```
make compile
```



Now that you have compiled the code you will now see a 'build' folder.
Place the pam_telegram.so that is found in the "build" folder wherever you feel is fit.
Make sure you chmod it as so

```
chmod 111 pam_telegram.so
```
This prevents tampering of any kind.

In /etc/pam.d/ssh (or whatever login prompt you want to enable this on)
Add this line
```
auth required	<PATH_TO_PAM_TELEGRAM_SO>/pam_telegram.so
```

If you want to make it so that a certain user does not have to follow this rule, you can modify this to be ..
```
auth [success=ok default=1] pam_succeed_if.so user != USER_YOU_WANT_TO_SKIP
auth required	<PATH_TO_PAM_TELEGRAM_SO>/pam_telegram.so
```


You should make this user require ssh key authentication ONLY so that it is secure. This is handy if the PAM module ever breaks and you need to repair it.
This prevents you from being locked out of your system.

Make sure that no other unwanted PAM modules are included so that you are not locked out of your account.
Save the file and close it.


In your sshd config (located at /etc/ssh/sshd_config)
you must add/change the following lines to enable ssh to use the PAM modules that you have specified

```
ChallengeResponseAuthentication yes
UsePAM yes
```
Make sure that these lines only occur once as the first instance of these options will be set for the rest of the config so any later declorations will be ignored.

Save the file and close it.

Finally, run this command to restart ssh and load the new settings (this will not close your current session)

```
sudo service sshd restart
```

This should now work like a charm!