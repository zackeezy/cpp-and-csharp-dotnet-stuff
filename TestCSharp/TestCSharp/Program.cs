using System;
using System.Collections.Generic;
using System.Collections;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Test;

namespace TestCSharp
{
    class Program
    {
        static void Main(string[] args)
        {
            if (!net.WSAInit())
            {
                return;
            }
            Console.WriteLine("Input Name: ");
            String name;
            name = Console.ReadLine();
            Console.Write("Would you like to be a Server (1) or Client(2), or would you just like to quit (3)?");
            String choice = Console.ReadLine();
            if (choice.Equals("1"))
            {
                Server s = new Server();
                s.mainServer(name);
            }
            else if (choice.Equals("2"))
            {
                Client c = new Client();
                List<PossibleServer> servers = new List<PossibleServer>();
                bool sock = net.initSock("", "", "udp");
                int numServers = c.getServers(net.getBroadcastAddress(), Networking.TicTacToe_UDPPORT, servers);
                c.mainClient(name, servers, numServers);
            }
            else
            {
                Console.WriteLine("K Bye.");
                return;
            }
            int cleanup = net.WSACleanupWrapper();
        }

        static public Networking net = new Networking();
    }

    class Server
    {
        private String buf = String.Copy("");

        private String host = String.Copy("");

        private String port = String.Copy("");

        private String response;
        public int mainServer(String playerName)
        {
            Program.net.initPassiveSock(Networking.TicTacToe_UDPPORT, "udp");

            Console.WriteLine("Waiting for a challenge...");

            host = Program.net.getBroadcastAddress();

            port = Networking.TicTacToe_UDPPORT;

            int len = Program.net.UDP_recv(buf, Networking.MAX_RECV_BUF, host, port);

            Boolean finished = false;
            while (!finished)
            {
                if (buf.Equals(Networking.TicTacToe_QUERY))
                {
                    response = String.Copy(Networking.TicTacToe_NAME + playerName);
                    Program.net.UDP_send(response, response.Length, host, port);
                }
                else if(buf.Contains(Networking.TicTacToe_CHALLENGE))
                {
                    String opponentName = buf.Substring(Networking.TicTacToe_CHALLENGE.Length);
                    Console.WriteLine("You are connected to " + opponentName);

                    //Add logic for the game here
                    bool myMove = false;
                    String message = null;
                    do
                    {
                        if (myMove)
                        {
                            Console.Write("Input chat message: ");
                            message = Console.ReadLine();
                            int len1 = Program.net.UDP_send(message, message.Length, host, port);
                        }
                        else
                        {
                            Console.WriteLine("Waiting on opponent...");
                            int status = Program.net.wait(Networking.WAIT_TIME, 0);
                            if (status > 0)
                            {
                                String recv = null;
                                int len2 = Program.net.UDP_recv(recv, Networking.MAX_RECV_BUF, host, port);
                                if (recv.ToLower() == "quit")
                                {
                                    Console.WriteLine("Your oppenet left.");
                                    break;
                                }
                                Console.WriteLine(opponentName + ": " + recv);
                            }
                        }
                    } while (message.ToLower() != "quit");
                    finished = true;
                }
            }

            return 0;
        }
    }

    class Client
    {
        private String buf = String.Copy("");

        private String host = String.Copy("");

        private String port = String.Copy("");

        private String response;
        public int mainClient(String playerName, List<PossibleServer> servers, int num)
        {
            Console.WriteLine("Sending challenge...");

            Console.WriteLine("There are " + num + " server(s).");
            if(num == 0)
            {
                Console.WriteLine("No Servers...");
                return 1;
            }
            for (int i = 0; i < num; i++)
            {
                int j = i + 1;
                Console.WriteLine(j + " " + servers[i].name);
            }
            String choice = Console.ReadLine();

            try
            {
                host = servers[Convert.ToInt32(choice) - 1].host;
                port = servers[Convert.ToInt32(choice) - 1].port;
            }
            catch(Exception e)
            {
                Console.WriteLine(e.Message);
            }

            Boolean finished = false;
            while (!finished)
            {
                if (buf.Equals(Networking.TicTacToe_QUERY))
                {
                    response = String.Copy(Networking.TicTacToe_CHALLENGE + playerName);
                    Program.net.UDP_send(response, response.Length, host, port);
                }
                else if (buf.Contains(Networking.TicTacToe_CHALLENGE))
                {
                    String opponentName = buf.Substring(Networking.TicTacToe_CHALLENGE.Length);
                    Console.WriteLine("You are connected to " + opponentName);

                    //Add logic for the game here
                    bool myMove = false;
                    do
                    {
                        if (myMove)
                        {
                            Console.Write("Input chat message: ");
                            buf = Console.ReadLine();
                            int len1 = Program.net.UDP_send(buf, buf.Length, host, port);
                        }
                        else
                        {
                            Console.WriteLine("Waiting on opponent...");
                            int status = Program.net.wait(Networking.WAIT_TIME, 0);
                            if (status > 0)
                            {
                                String recv = null;
                                int len2 = Program.net.UDP_recv(recv, Networking.MAX_RECV_BUF, host, port);
                                if (recv.ToLower() == "quit")
                                {
                                    Console.WriteLine("Your oppenet left.");
                                    break;
                                }
                                Console.WriteLine(opponentName + ": " + recv);
                            }
                        }
                    } while (buf.ToLower() != "quit");
                    finished = true;
                }
            }

            return 0;
        }

        public int getServers(String broadcastAddress, String broadcastPort, List<PossibleServer> servers)
        {
            int numServers = 0;

            int len3 = Program.net.UDP_send(Networking.TicTacToe_QUERY, Networking.TicTacToe_QUERY.Length, broadcastAddress, broadcastPort);

            int status = Program.net.wait(2, 0);
            if(status > 0)
            {
                String resp = null;

                int len4 = Program.net.UDP_recv(resp, Networking.MAX_RECV_BUF, broadcastAddress, broadcastPort);

                while(status > 0 && len4 > 0)
                {
                    if (resp.Contains(Networking.TicTacToe_NAME))
                    {
                        PossibleServer ps = new PossibleServer();
                        ps.name = resp.Substring(5);
                        ps.host = broadcastAddress;
                        ps.port = broadcastPort;
                        servers.Add(ps);
                    }

                    status = Program.net.wait(2, 0);
                    if(status > 0)
                    {
                        len4 = Program.net.UDP_recv(resp, Networking.MAX_RECV_BUF, broadcastAddress, broadcastPort);
                    }
                }
            }

            return numServers;
        }
    }

    class NimGame
    {
        public static void gameLogic()
        {

        }
    }

    class PossibleServer
    {
        public String name;

        public String host;

        public String port;
    }
}
