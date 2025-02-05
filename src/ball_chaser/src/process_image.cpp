#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    if (!client.call(srv)) {
        ROS_ERROR("Failed to call service command_robot");
    }
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{
    int white_pixel = 255;
    int left_boundary = img.width / 3;
    int right_boundary = 2 * img.width / 3;

    bool white_ball_found = false;
    int white_pixel_position = -1;

    // Loop through each pixel in the image and check if there's a bright white one
    for (int i = 0; i < img.height * img.step; i += 3) {
        if (img.data[i] == white_pixel && img.data[i + 1] == white_pixel && img.data[i + 2] == white_pixel) {
            white_ball_found = true;
            white_pixel_position = (i % img.step) / 3;
            break;
        }
    }

    // Drive the robot based on the position of the white ball
    if (white_ball_found) {
        if (white_pixel_position < left_boundary) {
            // Ball is on the left
            drive_robot(0.0, 0.5);
        } else if (white_pixel_position > right_boundary) {
            // Ball is on the right
            drive_robot(0.0, -0.5);
        } else {
            // Ball is in the center
            drive_robot(0.5, 0.0);
        }
    } else {
        // Request a stop when there's no white ball seen by the camera
        drive_robot(0.0, 0.0);
    }
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}

