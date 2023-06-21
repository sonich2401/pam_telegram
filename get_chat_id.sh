echo "Please enter your API token: "
read token

chat_id=$(curl "https://api.telegram.org/bot$token/getUpdates?limit=1&offset=-1" | sed -n -e 's/^.*chat\"\:{\"id\"\://p' | cut -d',' -f1)


echo "Your chat id is ..."
echo "$chat_id"
#rm a.tmp