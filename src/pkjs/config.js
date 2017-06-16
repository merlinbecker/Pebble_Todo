//@author merlinbecker
//@created 2017/04/27
//configured following the instructions under:
//https://github.com/pebble/clay

module.exports = [
  { 
    "type": "heading", 
    "defaultValue": "Todo API 0.1 configuration" 
  },
  {
  "type": "section",
  "items": [
  { 
    "type": "text", 
    "defaultValue": "Please enter your login data and the url to the api." 
  },
  {
      "type": "input",
      "messageKey": "apiUrl",
      "label": "URL to the TODO API",
      "defaultValue": "http://merl.be/todo_api/v0.1/"
  },  
  {
      "type": "input",
      "messageKey": "username",
      "label": "Your userame"
  },  
   {
      "type": "input",
      "messageKey": "password",
      "label": "Your password"
  },
  {
  "type": "submit",
  "defaultValue": "Save"
  }]}
];