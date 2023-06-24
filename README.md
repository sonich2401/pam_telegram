# pam_telegram
2FA for linux login via the terminal such as ssh, ftp, and normal login

# Why did I make this?
I got a Oracle Cloud Compute server and found many people trying to login as root. Root login was disabled,
but I just didnt feel safe enabling username/password authentication. SSH keys solve this issue but if
you are at someone's laptop and you need to login then you are stuck because your SSH key is at home.

This solves that issue by enabling password login and only allows people who you allow manually.


# How it works
When someone successfully enters their password, this module is called. It sends a message to the owner
of the server via telegram. It reports what user is trying to sign in and gives the owner a choice
of accepting or denying the user. If the user replies to the telegram bot with "/allow" then a authentication
code is sent via telegram that is to be entered into the terminal as a password. The user is then given
access to the system.

# How to install (Part 1)
Because getting your Telegram user id is not super simple I make a script to automate it. Just run

```
./install.sh
```

and it should walk you through how to install this module. You will need super user privledges to install
dependancies and create the directory for the PAM module.

If you would rather manually set it up (maybe because you do not have sudo but still want to compile) you can use the
'get_chat_id.sh' script although I do not recomend this as you will need sudo later to install the module. It is just better
to stick with the install.sh

# How to install (Part 2)
Setting up your PAM config is up to you. However, I recomend setting it up like so..

In /etc/pam.d/sshd (or whatever login prompt you want to enable this on)
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