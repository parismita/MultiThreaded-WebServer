#include "http_server.hh"
#include <vector>
#include <sys/stat.h>
#include <cstring>
#include <fstream>
#include <sstream>
#include <ctime> 
#include<string>    

//split by delim, store in array
vector<string> split(const string &s, char delim) {
  vector<string> elems;

  stringstream ss(s);
  string item;

  while (getline(ss, item, delim)) {
    if (!item.empty())
      elems.push_back(item);
  }

  return elems;
}

//this needs to be changed
string readFTS(const string &s) {
    stringstream ss(s);
    ifstream file(s);
    ss << file.rdbuf();
    return ss.str();
}

//constructor
HTTP_Request::HTTP_Request(string request) {
  this->HTTP_version = "1.0"; // We'll be using 1.0 irrespective of the request

  //empty req
  if(request.length()<1){
    this->method = "";
    this->url = "";
    this->flag = 1;
    return;
  }

  vector<string> lines = split(request, '\n');
  //empty line
  if(lines.size()<1){
    this->method = "";
    this->url = "";
    this->flag = 1;
    return;
  }

  vector<string> first_line = split(lines[0], ' ');
  
  /*
   TODO : extract the request method and URL from first_line here
  */
  if(first_line.size()>=2){
    this->method = first_line[0];
    this->url = first_line[1];
    this->flag = 0;
  }else{
    this->flag = 1;
    this->method = "";
    this->url = "";
    return;
  }
  

  struct stat sb;
  if (this->method != "GET") {
    cerr << "Method '" << this->method << "' not supported" << endl;
    this->flag = 1;
    //exit(1);
  }
}

//handle request funciton
HTTP_Response *handle_request(string req) {
  HTTP_Request *request = new HTTP_Request(req);
  HTTP_Response *response = new HTTP_Response();
  response->HTTP_version = "1.0";


  //handling space
  string url = "html_files"+request->url;
  //cout<<url<<endl;
  
  struct stat sb;
  //cout<<url<<stat(url.c_str(), &sb);
  if (stat(url.c_str(), &sb) == 0 && request->flag!=1) // requested path exists
  {
    //cout<<url.c_str();
    response->status_code = "200";
    response->status_text = "OK";
    response->content_type = "text/html";
    
    //cout<<S_ISDIR(sb.st_mode);

    if (S_ISDIR(sb.st_mode)) {
      /*
      In this case, requested path is a directory.
      TODO : find the index.html file in that directory (modify the url
      accordingly)
      */
      if(url.find("index.html")>url.length()&&url[url.length()-1]=='/'){
        url = url + "index.html";
      }
      else if(url.find("index.html")>url.length()&&url[url.length()-1]!='/'){
        url = url + "/index.html";
      }
      //cout<<url;
    }

    /*
    TODO : open the file and read its contents
    */
    response->body= readFTS(url);
    //cout<<response->body;
    //file.close();
    response->content_length=to_string(response->body.length());
    //cout<<(response->body).length();

    /*
    TODO : set the remaining fields of response appropriately
    */
  }

  else {
    //cout<<"404 area\n";
    response->status_code = "404";
    response->status_text = "Not Found";
    response->content_type = "text/html";
    response->content_length="3";
    response->body="404";

    /*
    TODO : set the remaining fields of response appropriately
    */
  }

  delete request;

  return response;
}

string HTTP_Response::get_string(HTTP_Response *res) {
  /*
  TODO : implement this function
  */
  //cout<<res->content_length<<res->body<<"a";
 return("HTTP/"+res->HTTP_version+" "+
  res->status_code+" "+  // ex: 200, 404, etc.
  res->status_text+"\n"+ 
  "Content-Length:"+res->content_length+"\n"+
  "Content-Type: "+res->content_type+"\n\n"+res->body+"\n\n");
}

/*
HTTP/1.1 200 OK
Date: Mon, 27 Jul 2009 12:28:53 GMT
Server: Apache/2.2.14 (Win32)
Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT
Content-Length: 88
Content-Type: text/html
Connection: Closed
*/