#include <iostream>
#include <fstream>
#include <pqxx/pqxx>

#include "exerciser.h"

using namespace std;
using namespace pqxx;

void insertState(string file,connection *C){
  std::ifstream f;
  f.open(file.c_str(), std::ifstream::in);
  string temp;
  while (std::getline(f, temp)) {
    size_t space=temp.find(" ");
    string id=temp.substr(0,space);
    size_t Return=temp.find("\n");
    string name=temp.substr(space+1,Return-space-1);
    add_state(C,name);
    /*
    string insert="INSERT INTO \"STATE\" VALUES ('"+id+"', '"+name+"');";
    //cout<<insert<<endl;
    work W(*C);
    W.exec(insert);
    W.commit();*/
  }
  f.close();
}
void insertColor(string file,connection *C){
  std::ifstream f;
  f.open(file.c_str(), std::ifstream::in);
  string temp;
  while (std::getline(f, temp)) {
    size_t space=temp.find(" ");
    string id=temp.substr(0,space);
    size_t Return=temp.find("\n");
    string color=temp.substr(space+1,Return-space-1);
    add_color(C,color);
    /*
    string insert="INSERT INTO \"COLOR\" VALUES ('"+id+"', '"+color+"');";
    work W(*C);
    W.exec(insert);
    W.commit();*/
  }
  f.close();
}
void insertTeam(string file,connection *C){
  std::ifstream f;
  f.open(file.c_str(), std::ifstream::in);
  string temp;
  while (std::getline(f, temp)) {
    size_t space1=temp.find(" ");
    string teamid=temp.substr(0,space1);
    size_t space2=temp.find(" ",space1+1);
    string name =temp.substr(space1+1,space2-space1-1);
    size_t space3=temp.find(" ",space2+1);
    string stateid=temp.substr(space2+1,space3-space2-1);
    size_t space4=temp.find(" ",space3+1);
    string colorid=temp.substr(space3+1,space4-space3-1);
    size_t space5=temp.find(" ",space4+1);
    string Wins=temp.substr(space4+1,space5-space4-1);
    size_t Return=temp.find("\n",space5+1);
    string loss=temp.substr(space5+1,Return-space5-1);
    int state_id=std::stoi(stateid);
    int color_id=std::stoi(colorid);
    int wins=std::stoi(Wins);
    int losses=std::stoi(loss);
    add_team(C,name,state_id,color_id,wins,losses);
    /*
    string insert="INSERT INTO \"TEAM\" VALUES ('"+teamid+"', '"+name+"', '"+stateid+"', '"+colorid+"', '"+wins+"', '"+loss+"');";
    work W(*C);
    W.exec(insert);
    W.commit();*/
  }
  f.close();
}
void insertPlayer(string file,connection *C){
  std::ifstream f;
  f.open(file.c_str(), std::ifstream::in);
  string temp;
  
  while (std::getline(f, temp)) {
    size_t space1=temp.find(" ");
    string playerid=temp.substr(0,space1);
    size_t space2=temp.find(" ",space1+1);
    string teamid=temp.substr(space1+1,space2-space1-1);
    size_t space3=temp.find(" ",space2+1);
    string uniform_num=temp.substr(space2+1,space3-space2-1);
    size_t space4=temp.find(" ",space3+1);
    string first=temp.substr(space3+1,space4-space3-1);
    size_t space5=temp.find(" ",space4+1);
    string last=temp.substr(space4+1,space5-space4-1);
    size_t space6=temp.find(" ",space5+1);
    string MPG=temp.substr(space5+1,space6-space5-1);
    size_t space7=temp.find(" ",space6+1);
    string PPG=temp.substr(space6+1,space7-space6-1);
    size_t space8=temp.find(" ",space7+1);
    string RPG=temp.substr(space7+1,space8-space7-1);
    size_t space9=temp.find(" ",space8+1);
    string APG=temp.substr(space8+1,space9-space8-1);
    size_t space10=temp.find(" ",space9+1);
    string SPG=temp.substr(space9+1,space10-space9-1);
    size_t Return=temp.find("\n",space10+1);
    string BPG=temp.substr(space10+1,Return-space10-1);
    int team_id=std::stoi(teamid);
    int jersey_num=std::stoi(uniform_num);
    int mpg=std::stoi(MPG);
    int ppg=std::stoi(PPG);
    int rpg=std::stoi(RPG);
    int apg=std::stoi(APG);
    double spg=std::stod(SPG);
    double bpg=std::stod(BPG);
    add_player(C, team_id,jersey_num,first, last,mpg, ppg,  rpg,  apg, spg,  bpg);
    
    /*
    work W(*C);
    string insert="INSERT INTO \"PLAYER\" VALUES ('"+playerid+"', '"+teamid+"', '"+uniform_num+"', "+W.quote(first)+", "+W.quote(last)+", '"+MPG+"', '"+PPG+"', '"+RPG+"', '"+APG+"', '"+SPG+"', '"+BPG+"');";
        W.exec(insert);
	W.commit();*/
  }
  f.close();
}
int main (int argc, char *argv[]) 
{

  //Allocate & initialize a Postgres connection object
  connection *C;

  try{
    //Establish a connection to the database
    //Parameters: database name, user name, user password
    C = new connection("dbname=ACC_BBALL user=postgres password=passw0rd");
    
    if (C->is_open()) {
      cout << "Opened database successfully: " << C->dbname() << endl;
    } else {
      cout << "Can't open database" << endl;
      return 1;
    }
  } catch (const std::exception &e){
    cerr << e.what() << std::endl;
    return 1;
  }


  //TODO: create PLAYER, TEAM, STATE, and COLOR tables in the ACC_BBALL database
  //      load each table with rows from the provided source txt files
  string delete1="DROP TABLE IF EXISTS \"PLAYER\" CASCADE;";
  work Wd1(*C);
  Wd1.exec(delete1);
  Wd1.commit();
  string delete2="DROP TABLE IF EXISTS \"TEAM\" CASCADE;";
  work Wd2(*C);
  Wd2.exec(delete2);
  Wd2.commit();
  string delete3="DROP TABLE IF EXISTS \"STATE\" CASCADE;";
  work Wd3(*C);
  Wd3.exec(delete3);
  Wd3.commit();
  string delete4="DROP TABLE IF EXISTS \"COLOR\" CASCADE;";
  work Wd4(*C);
  Wd4.exec(delete4);
  Wd4.commit();


  string sql1="CREATE TABLE \"STATE\" (\"STATE_ID\" SERIAL PRIMARY KEY, \"NAME\" varchar(512));";
  work Wc1(*C);
  Wc1.exec(sql1);
  Wc1.commit();
  string sql2="CREATE TABLE \"COLOR\" (\"COLOR_ID\" SERIAL PRIMARY KEY, \"NAME\" varchar(512));";
  work Wc2(*C);
  Wc2.exec(sql2);
  Wc2.commit();
  string sql3="CREATE TABLE \"TEAM\" (\"TEAM_ID\" SERIAL PRIMARY KEY, \"NAME\" varchar(512), \"STATE_ID\" int, \"COLOR_ID\" int, \"WINS\" int, \"LOSSES\" int, CONSTRAINT \"STATE_FK\" FOREIGN KEY (\"STATE_ID\") REFERENCES \"STATE\"(\"STATE_ID\") ON DELETE SET NULL ON UPDATE CASCADE, CONSTRAINT \"COLOR_FK\" FOREIGN KEY (\"COLOR_ID\") REFERENCES \"COLOR\"(\"COLOR_ID\") ON DELETE SET NULL ON UPDATE CASCADE);";
  work Wc3(*C);
  Wc3.exec(sql3);
  Wc3.commit();
  string sql4="CREATE TABLE \"PLAYER\" (\"PLAYER_ID\" SERIAL PRIMARY KEY,\"TEAM_ID\" int, \"UNIFORM_NUM\" int, \"FIRST_NAME\" varchar(512), \"LAST_NAME\" varchar(512), \"MPG\" int,\"PPG\" int,\"RPG\" int,\"APG\" int,\"SPG\" float8,\"BPG\" float8,CONSTRAINT \"TEAM_FK\" FOREIGN KEY (\"TEAM_ID\") REFERENCES \"TEAM\"(\"TEAM_ID\") ON DELETE SET NULL ON UPDATE CASCADE);";
  work Wc4(*C);
  Wc4.exec(sql4);
  Wc4.commit();

  insertState("state.txt",C);
  insertColor("color.txt",C);
  insertTeam("team.txt",C);
  insertPlayer("player.txt",C);
  exercise(C);


  //Close database connection
  C->disconnect();

  return 0;
}


