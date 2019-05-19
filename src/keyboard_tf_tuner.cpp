#include <ros/ros.h>
#include <tf/transform_broadcaster.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>

struct termios initial_settings, new_settings;

// Transformation message
geometry_msgs::TransformStamped tf_trans;
float xpi, ypi, zpi, rri, pri, yri;
float xp = 0, yp = 0, zp = 0, rr = 0, pr = 0, yr = 0;
float delta_p = 0.05;
float delta_r = 0.05;
bool position_mode = false;
bool generic_command = false;
bool printed = false;

// Transform stuff
tf::TransformBroadcaster* tf_broadcaster;

/*
 * Timer callback
 */
void timerCallback(const ros::TimerEvent&)
{
  // Fill the transform
  tf_trans.transform.rotation = tf::createQuaternionMsgFromRollPitchYaw(rr, pr, yr);
  tf_trans.transform.translation.x = xp;
  tf_trans.transform.translation.y = yp;
  tf_trans.transform.translation.z = zp;

  tf_trans.header.stamp = ros::Time::now();

  // publish the transform
  tf_broadcaster->sendTransform(tf_trans);

  // Spin
  ros::spinOnce();
}

/*
 * Main
 */
int main(int argc, char** argv)
{

  /*
   * Get the input arguments
   */
  if (argc == 9)
  {
    // Get the arguments
    xpi = xp = atof(argv[1]);
    ypi = yp = atof(argv[2]);
    zpi = zp = atof(argv[3]);

    rri = rr = atof(argv[4]);
    pri = pr = atof(argv[5]);
    yri = yr = atof(argv[6]);

    std::string parent_frame_name = argv[7];
    std::string child_frame_name = argv[8];

    tf_trans.header.frame_id = parent_frame_name;
    tf_trans.child_frame_id = child_frame_name;

    std::cout << "parent frame = " << parent_frame_name << std::endl;
    std::cout << "child frame = " << child_frame_name << std::endl;
    std::cout << "  x = " << xp << std::endl;
    std::cout << "  y = " << yp << std::endl;
    std::cout << "  z = " << zp << std::endl;
    std::cout << "  r = " << rr << std::endl;
    std::cout << "  p = " << pr << std::endl;
    std::cout << "  y = " << yr << std::endl << std::endl;
  }
  else
  {
    std::cout << "Invalid amount of input arguments. Expecting: 'x' 'y' 'z' 'R' 'P' 'Y' 'parent_frame' 'child_frame'" << std::endl;
    return 1;
  }

  /*
   * Make getchar nonblocking
   */
  tcgetattr(0, &initial_settings); /* grab old terminal i/o settings */
  new_settings = initial_settings;
  new_settings.c_lflag &= ~ICANON; /* disable buffered i/o */
  new_settings.c_lflag = ECHO;
  tcsetattr(0, TCSANOW, &new_settings); /* use these new terminal i/o settings now */

  /*
   * Print out the keys
   */
  std::cout << "Key layout: " << std::endl << std::endl;
  std::cout << "  * q/w: decrease/increase x/roll \n";
  std::cout << "  * a/s: decrease/increase y/pitch \n";
  std::cout << "  * q/w: decrease/increase y/yaw \n\n";
  std::cout << "  * c: toggle between position/rotation modes \n";
  std::cout << "  * p: print out the transformation \n";
  std::cout << "  * r: reset the transformation \n";
  std::cout << "  * m: change step sizes \n";
  std::cout << "  * e: exit the program \n\n";

  /*
   * Initialize the ros stuff
   */
  ros::init(argc, argv, "keyboard_tf_tuner");
  ros::NodeHandle nh;
  tf_broadcaster = new tf::TransformBroadcaster();

  // Create a tf publishing timer
  ros::Timer timer = nh.createTimer(ros::Duration(0.1), timerCallback);

  ros::AsyncSpinner spinner(2); // Use 2 threads
  spinner.start();

  /*
   * Main loop
   */
  while (ros::ok())
  {
    //
    if (!printed)
    {
      position_mode ? (std::cout << "position <-: ") : (std::cout << "rotation <-: ");
      printed = true;
    }

    // Non blocking call to the get char
    char character = std::getchar();

    // Sleep if no input was received
    if (character == -1)
    {
      ros::Duration(0.2).sleep();
      continue;
    }

    // New line, otherwise it will be a mess
    std::cout << std::endl;

    // Check if any generic characters were typed
    switch(character)
    {
      case 'c': // Change the input mode (position or rotation)
        position_mode = !position_mode;
        generic_command = true;
        break;

      case 'p': // Print out the transform
        std::cout << "Printing the transform: \n" << tf_trans.transform;
        std::cout << "Rotation in RPY:" << std::endl;
        std::cout << "  r: " << rr << std::endl;
        std::cout << "  p: " << pr << std::endl;
        std::cout << "  y: " << yr << std::endl;

        generic_command = true;
        break;

      case 'm': // Change the delta values

        // First, make the getline blocking
        tcsetattr(0, TCSANOW, &initial_settings);

        try
        {
          std::cin.clear();
          std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
          std::string data;

          std::cout << "\nenter position step size (m): ";
          std::getline(std::cin, data);
          delta_p = std::stof(data);

          std::cout << "enter rotation step size (rad): ";
          std::getline(std::cin, data);
          delta_r = std::stof(data);

        }
        catch(...)
        {
          std::cout << "Invalid input, better luck next time \n";
        }

        // Make the getline non-blocking
        tcsetattr(0, TCSANOW, &new_settings);

        generic_command = true;
        break;

      case 'r': // Reset the transform

        std::cout << "\nresetting the x y z R P Y \n";
        xp = xpi;
        yp = ypi;
        zp = zpi;
        rr = rri;
        pr = pri;
        yr = yri;

        generic_command = true;

      case 'e': // Exit the program
        std::cout << "Exiting\n";
        tcsetattr(0, TCSANOW, &initial_settings);
        return 0;
    }

    // Dont proceed if a generic command was received
    if (generic_command)
    {
      generic_command = false;
      printed = false;
      continue;
    }

    /* TUNING CASES */
    switch(character)
    {
      case 'w': // Increase x/r
        position_mode ? (xp += delta_p) : (rr += delta_r);
        break;

      case 'q': // Decrease x/r
        position_mode ? (xp -= delta_p) : (rr -= delta_r);
        break;

      case 's': // Increase y/p
        position_mode ? (yp += delta_p) : (pr += delta_r);
        break;

      case 'a': // Decrease y/p
        position_mode ? (yp -= delta_p) : (pr -= delta_r);
        break;

      case 'x': // Increase z/y
        position_mode ? (zp += delta_p) : (yr += delta_r);
        break;

      case 'z': // Decrease z/y
        position_mode ? (zp -= delta_p) : (yr -= delta_r);
        break;

      default:
        std::cout << "Received an unknown character" << std::endl;
    }

    // Spin
    ros::spinOnce();

    printed = false;
  }

  // Release the memory
  delete tf_broadcaster;

  // Reset the terminal settings
  tcsetattr(0, TCSANOW, &initial_settings);
}
