/**************************************************************************************************
 * @ File: main.cpp
 * @ Author: MarkGao
 * @ Date: 2020-05-12
 * @ Email: 819699632@qq.com
 * @ Version: 1.0
 * @ Description: System MAIN function
**************************************************************************************************/
#include <math.h>
#include <uWS/uWS.h>
#include <iostream>
#include <string>
#include "json.hpp"
#include "PID.h"

// For convenience
using nlohmann::json;
using std::string;
using std::cout;
using std::endl;

// For converting back and forth between radians and degrees.
constexpr double pi() { return M_PI; }
double deg2rad(double x) { return x * pi() / 180; }
double rad2deg(double x) { return x * 180 / pi(); }



/**************************************************************************************************
 * @ Function: hasData()
 * @ Description: Checks if the SocketIO event has JSON data.
 * @ Calls:
 * @ Input: string s
 * @ Output: void
 * @ Return: If there is data, the JSON object in string format will be returned,
 *           else the empty string "" will be returned.
 * @ Others:
**************************************************************************************************/
string hasData(string s)
{
    auto found_null = s.find("null");
    auto b1 = s.find_first_of("[");
    auto b2 = s.find_last_of("]");

    if(found_null != string::npos)
    {
        return "";
    }
    else if(b1 != string::npos && b2 != string::npos)
    {
        return s.substr(b1, b2 - b1 + 1);
    }

    return "";
}



/**************************************************************************************************
 * @ Function: main()
 * @ Description:
 * @ Calls:
 * @ Input: void
 * @ Output: void
 * @ Return: void
 * @ Others:
**************************************************************************************************/
int main()
{
    uWS::Hub h;
    PID pid;

    // TODO: Set the initial coefficients.
    double k_p = 0.128;
    double k_i = 0.0025;
    double k_d = 1.35;

    // TODO: Initialize the pid variables.
    pid.Init(k_p, k_i, k_d);


    h.onMessage([&pid](uWS::WebSocket<uWS::SERVER> ws,
                char *data,
                size_t length,
                uWS::OpCode opCode)
    {
        // "42" at the start of the message means there's a websocket message event.
        // The 4 signifies a websocket message
        // The 2 signifies a websocket event
        if(length && length > 2 && data[0] == '4' && data[1] == '2')
        {
            auto s = hasData(string(data).substr(0, length));

            if(s != "")
            {
                auto j = json::parse(s);

                string event = j[0].get<string>();

                if(event == "telemetry")
                {
                    // j[1] is the data JSON object
                    double cte = std::stod(j[1]["cte"].get<string>());
                    double speed = std::stod(j[1]["speed"].get<string>());
                    double angle = std::stod(j[1]["steering_angle"].get<string>());
                    double steer_value;

                    /**
                     * TODO: Calculate steering value here, remember the steering value is [-1, 1].
                     * NOTE: Feel free to play around with the throttle and speed.
                     *       Maybe use another PID controller to control the speed!
                     */

                    pid.UpdateError(cte);

                    steer_value = pid.TotalError();

                    // DEBUG
                    cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n"
                              << "CTE: " << cte
                              << "\nSteering Value: " << steer_value
                              << endl;
//                    cout<<"!!! "<<j[1]['x'].get<string>()<<endl;
//                    cout<<"!!! "<<j[1]['y'].get<string>()<<endl;

                    json msgJson;
                    msgJson["steering_angle"] = steer_value;
                    msgJson["throttle"] = 0.3;
                    auto msg = "42[\"steer\"," + msgJson.dump() + "]";

                    ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
                }  // end "telemetry" if
            }
            else
            {
                // Manual driving
                string msg = "42[\"manual\",{}]";
                ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
            }    // end s != ""
        }    // end websocket message if
    });    // end h.onMessage


    // Connect to simulator
    h.onConnection([&h](uWS::WebSocket<uWS::SERVER> ws, uWS::HttpRequest req)
    {
        cout<<"!!! Connected !!!"<<endl;
    });


    // Disconnect to simulator
    h.onDisconnection([&h](uWS::WebSocket<uWS::SERVER> ws, int code, char *message, size_t length)
    {
        ws.close();
        cout<<"!!! Disconnected !!!"<<endl;
    });


    // Listening to port
    int port = 4567;
    if(h.listen(port))
    {
        cout<<"Listening to port "<<port<<endl;
    }
    else
    {
        std::cerr<<"Failed to listen to port"<<endl;
        return -1;
    }
  

    h.run();
}
