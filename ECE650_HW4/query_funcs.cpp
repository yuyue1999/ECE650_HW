#include "query_funcs.h"
#include <iomanip>
void add_player(connection *C, int team_id, int jersey_num, string first_name, string last_name,
                int mpg, int ppg, int rpg, int apg, double spg, double bpg)
{
  work W(*C);
  string insert="INSERT INTO \"PLAYER\" (\"TEAM_ID\",\"UNIFORM_NUM\",\"FIRST_NAME\",\"LAST_NAME\",\"MPG\",\"PPG\",\"RPG\",\"APG\",\"SPG\",\"BPG\") VALUES ("+W.quote(team_id)+","+W.quote(jersey_num)+","+W.quote(first_name)+","+W.quote(last_name)+","+W.quote(mpg)+","+W.quote(ppg)+","+W.quote(rpg)+","+W.quote(apg)+","+W.quote(spg)+","+W.quote(bpg)+");";
  W.exec(insert);
  W.commit();
}


void add_team(connection *C, string name, int state_id, int color_id, int wins, int losses)
{
  work W(*C);
  string insert="INSERT INTO \"TEAM\" (\"NAME\",\"STATE_ID\",\"COLOR_ID\",\"WINS\",\"LOSSES\") VALUES ("+W.quote(name)+","+W.quote(state_id)+","+W.quote(color_id)+","+W.quote(wins)+","+W.quote(losses)+");";
  
  W.exec(insert);
  W.commit();
}


void add_state(connection *C, string name)
{work W(*C);
  string insert="INSERT INTO \"STATE\" (\"NAME\") VALUES ("+W.quote(name)+");";
    W.exec(insert);
    W.commit();
}


void add_color(connection *C, string name)
{
  work W(*C);
  string insert="INSERT INTO \"COLOR\" (\"NAME\") VALUES ("+W.quote(name)+");";
  W.exec(insert);
  W.commit();
}

/*
 * All use_ params are used as flags for corresponding attributes
 * a 1 for a use_ param means this attribute is enabled (i.e. a WHERE clause is needed)
 * a 0 for a use_ param means this attribute is disabled
 */
void query1(connection *C,
	    int use_mpg, int min_mpg, int max_mpg,
            int use_ppg, int min_ppg, int max_ppg,
            int use_rpg, int min_rpg, int max_rpg,
            int use_apg, int min_apg, int max_apg,
            int use_spg, double min_spg, double max_spg,
            int use_bpg, double min_bpg, double max_bpg
            )
{
  string insert="SELECT * FROM \"PLAYER\"";
  int where=0;
  if(use_mpg==1){
    if(where==0){
      insert=insert+" WHERE ";
    }else{
      insert=insert+" AND ";
    }
    insert=insert+"(\"MPG\" BETWEEN "+std::to_string(min_mpg)+" AND "+std::to_string(max_mpg)+" )";
    where=1;
  }
  if(use_ppg==1){
    if(where==0){
      insert=insert+" WHERE ";
    }else{
      insert=insert+" AND ";
    }
    insert=insert+"(\"PPG\" BETWEEN "+std::to_string(min_ppg)+" AND "+std::to_string(max_ppg)+" )";
    where=1;
  }
  if(use_rpg==1){
    if(where==0){
      insert=insert+" WHERE ";
    }else{
      insert=insert+" AND ";
    }
    insert=insert+"(\"RPG\" BETWEEN "+std::to_string(min_rpg)+" AND "+std::to_string(max_rpg)+" )";
    where=1;
  }
  if(use_apg==1){
    if(where==0){
      insert=insert+" WHERE ";
    }else{
      insert=insert+" AND ";
    }
    insert=insert+"(\"APG\" BETWEEN "+std::to_string(min_apg)+" AND "+std::to_string(max_apg)+" )";
    where=1;
  }
  if(use_spg==1){
    if(where==0){
      insert=insert+" WHERE ";
    }else{
      insert=insert+" AND ";
    }
    insert=insert+"(\"SPG\" BETWEEN "+std::to_string(min_spg)+" AND "+std::to_string(max_spg)+" )";
    where=1;
  }
  if(use_bpg==1){
    if(where==0){
      insert=insert+" WHERE ";
    }else{
      insert=insert+" AND ";
    }
    insert=insert+"(\"BPG\" BETWEEN "+std::to_string(min_bpg)+" AND "+std::to_string(max_bpg)+" )";
    where=1;
  }
  insert=insert+";";
  nontransaction N(*C);
  result R(N.exec(insert));
  cout << "PLAYER_ID TEAM_ID UNIFORM_NUM FIRST_NAME LAST_NAME MPG PPG RPG APG SPG BPG"<<endl;
  for (result::const_iterator r = R.begin(); r != R.end(); ++r) {
    cout << r[0].as<int>() << " " << r[1].as<int>() << " " << r[2].as<int>();
    cout << " " << r[3].as<string>() << " " << r[4].as<string>();
    cout << " "<< r[5].as<int>() << " " << r[6].as<int>() << " " << r[7].as<int>()<< " " << r[8].as<int>() << " ";
    cout.setf(ios::fixed, ios::floatfield);
    cout.precision(1);
    cout << r[9].as<double>() << " " << r[10].as<double>() <<endl;
  }
}


void query2(connection *C, string team_color)
{
  nontransaction N(*C);
  string insert = "SELECT \"TEAM\".\"NAME\" FROM \"TEAM\", \"COLOR\" WHERE \"TEAM\".\"COLOR_ID\" = \"COLOR\".\"COLOR_ID\" AND \"COLOR\".\"NAME\" = " +N.quote(team_color) + ";";
  result R(N.exec(insert));
  cout << "NAME"<<endl;
  for (result::const_iterator r = R.begin(); r != R.end(); ++r) {
    cout << r[0].as<string>() << endl;
  }
}


void query3(connection *C, string team_name)
{
  nontransaction N(*C);
  string insert= "SELECT \"FIRST_NAME\", \"LAST_NAME\" FROM \"PLAYER\", \"TEAM\" WHERE \"PLAYER\".\"TEAM_ID\"=\"TEAM\".\"TEAM_ID\" AND \"TEAM\".\"NAME\" = "+N.quote(team_name);
  insert=insert+" ORDER BY \"PLAYER\".\"PPG\" DESC;";
  result R(N.exec(insert));
  cout << "FIRST_NAME LAST_NAME"<<endl;
  for (result::const_iterator r = R.begin();r != R.end(); ++r) {
    cout << r[0].as<string>() << " "<< r[1].as<string>() << endl;
  }
}


void query4(connection *C, string team_state, string team_color)
{
  nontransaction N(*C);
  string insert="SELECT \"UNIFORM_NUM\", \"FIRST_NAME\", \"LAST_NAME\" FROM \"PLAYER\", \"STATE\", \"COLOR\", \"TEAM\" WHERE \"PLAYER\".\"TEAM_ID\"=\"TEAM\".\"TEAM_ID\" AND \"TEAM\".\"STATE_ID\"=\"STATE\".\"STATE_ID\" AND \"TEAM\".\"COLOR_ID\"=\"COLOR\".\"COLOR_ID\" AND \"COLOR\".\"NAME\" = " + N.quote(team_color) + " AND ";
  insert=insert+ "\"STATE\".\"NAME\"=" + N.quote(team_state) + ";";
  result R(N.exec(insert));
  cout << "UNIFORM_NUM FIRST_NAME LAST_NAME"<<endl;
  for (result::const_iterator r = R.begin(); r != R.end(); ++r) {
    cout << r[0].as<int>() << " "<< r[1].as<string>() << " "<< r[2].as<string>() << endl;
  }
}


void query5(connection *C, int num_wins)
{
  nontransaction N(*C);
  string insert;
  insert= "SELECT \"PLAYER\".\"FIRST_NAME\", \"PLAYER\".\"LAST_NAME\", \"TEAM\".\"NAME\", \"TEAM\".\"WINS\" FROM \"PLAYER\", \"TEAM\" WHERE \"PLAYER\".\"TEAM_ID\"=\"TEAM\".\"TEAM_ID\" AND \"TEAM\".\"WINS\">" + std::to_string(num_wins) + ";";
  result R(N.exec(insert));
  cout << "FIRST_NAME LAST_NAME NAME WINS"<<endl;
  for (result::const_iterator r = R.begin(); r != R.end(); ++r) {
    cout << r[0].as<string>() << " "<<r[1].as<string>() << " "<< r[2].as<string>() << " "<< r[3].as<int>() << endl;
  }
}
