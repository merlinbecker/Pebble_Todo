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