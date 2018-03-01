#include "config.h"
#include "login.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>

/*
 * Need to Add:
 * - Regeneration of cookies on the fly?
 * - Better CSS
 * - Move large HTML files to their own file.
 */

/*Convert to a file and serve file?*/
const String loginPage = "<!DOCTYPE html><html><head><title>Login</title></head><body> <div id=\"login\"> <form action='/login' method='POST'> <center> <h1>Login </h1><p><input type='text' name='user' placeholder='User name'></p><p><input type='password' name='pass' placeholder='Password'></p><br><button type='submit' name='submit'>login</button></center> </form></body></html>";

String cookieChars = "abcdefghijklmnopqrstuvwxyz0123456789", basicUsername = "user", basicPassword = "password", adminUsername = "admin", adminPassword = "admin";
String userCookie, adminCookie;

//Generate cookies on start
void generateSession(){
  userCookie = generateCookie(userCookie);
  adminCookie = generateCookie(adminCookie);
  Serial.print("User Cookie = ");
  Serial.println(userCookie);
  Serial.print("Admin Cookie = ");
  Serial.println(adminCookie);
}

//Serve 404 page
void handleNotFound(){
  server.send(404, "text/plain", "404: Not found");
}

//Check correct user cookie in place and direct to root
void handleRoot(){
  if (!checkUserAuth()){
    String header = "HTTP/1.1 301 OK\r\nLocation: /login\r\nCache-Control: no-cache\r\n\r\n";
    server.sendContent(header);
    return;
  }
  //Remote code inserted here
  server.send(200, "text/html", "You reached the User Page <a href='/logout'>Logout</a>");
}

//Check correct admin cookie in place and direct to admin page
void handleAdmin(){
  Serial.println("in admin handler");
  if(!checkAdminAuth()){
    String header = "HTTP/1.1 301 OK\r\nLocation: /login\r\nCache-Control: no-cache\r\n\r\n";
    server.sendContent(header);
    return;
  }
  server.send(200, "text/html", "You reached the Admin Page <a href='/logout'>Logout</a>");
}

//Post login form and handle response
void handleLogin(){
  Serial.println("in login handler");
  //Check if login info has been posted
  if(server.hasArg("user") && server.hasArg("pass")){
    //Check if basic user details correct
    if(server.arg("user") == basicUsername && server.arg("pass") == basicPassword){
      //Set user cookie and redirect to root
      server.sendHeader("Set-Cookie", "user=" + userCookie);
      server.sendHeader("Set-Cookie", "admin=0");
      server.sendHeader("Location","/");
      server.sendHeader("Cache-Control","no-cache");
      server.send(301);
      return;
    }
    //Check if admin user details correct
    else if(server.arg("user") == adminUsername && server.arg("pass") == adminPassword){
      //Set admin and user cookes and direct to Admin page
      server.sendHeader("Set-Cookie", "user=" + userCookie);
      server.sendHeader("Set-Cookie", "admin=" + adminCookie);
      server.sendHeader("Location","/admin");
      server.sendHeader("Cache-Control","no-cache");
      server.send(301);
      return;
    }
  }

  //Send login page here
  server.send(200, "text/html", loginPage);
}

//Set admin and user to 0 & redirect to login
void handleLogout(){
  server.sendHeader("Set-Cookie", "user=0");
  server.sendHeader("Set-Cookie", "admin=0");
  server.sendHeader("Location","/login");
  server.sendHeader("Cache-Control","no-cache");
  server.send(301)
}

//Itterate through alpha-num to generate 32 bit cookie
String generateCookie(String cookie){
  cookie = "";
  for( uint8_t i = 0; i < 32; i++) cookie += cookieChars[random(0, cookieChars.length())];
  return cookie;
}

/*Make more complex to handle both use and admin auth?*/
//Check User Cookie
bool checkUserAuth(){
  if (server.hasHeader("Cookie")){
    String receivedCookie = server.header("Cookie");
    String compareCookie = "user=" + userCookie;
    if (receivedCookie.indexOf(compareCookie) != -1){
      return true;
    }
  }
  return false;
}

// Check Admin Cookie
bool checkAdminAuth(){
  if (server.hasHeader("Cookie")){
    String receivedCookie = server.header("Cookie");
    String compareCookie = "admin=" + adminCookie;
    if (receivedCookie.indexOf(compareCookie) != -1){
      return true;
    }
  }
  return false;
}
