#include <stdio.h>
#include <iostream>
#include "client_terminal.h"
#include "set_piece.h"


SetPiece::SetPiece(std::vector<std::string>& image, bool init_is_bold, AnsiColor init_color) : piece_id(0), image(image), is_bold(init_is_bold), color(init_color) {
    int image_width = image[0].length();
    int image_height = image.size();
    bounding_box = BoundingBox(1,1,image_width+1, image_height+1);
    next_bounding_box = BoundingBox(1+x_speed, 1+y_speed, image_width+1+x_speed, image_height+1+y_speed);
}

void  SetPiece::set_speed(float new_x_speed, float new_y_speed) {
    x_speed = new_x_speed;
    y_speed = new_y_speed;
    next_bounding_box.set(bounding_box.getXMin()+x_speed, bounding_box.getYMin()+y_speed);
}
void SetPiece::invert_x_speed() {
    x_speed = -x_speed;
    next_bounding_box.set(bounding_box.getXMin()+x_speed, bounding_box.getYMin()+y_speed);
}
void SetPiece::invert_y_speed() {
    y_speed = -y_speed;
    next_bounding_box.set(bounding_box.getXMin()+x_speed, bounding_box.getYMin()+y_speed);
}

void SetPiece::set_position(int x, int y) {
    bounding_box.set(x, y);
    next_bounding_box.set(x+x_speed, y+y_speed);
}

void SetPiece::update_position() {
    bounding_box.move(x_speed, y_speed);
    next_bounding_box.move(x_speed, y_speed);
}

// void SetPiece::draw() {
//     // Add these debug lines at the start of draw()
//     //std::cout << "is_bold: " << is_bold << std::endl;
//     //std::cout << "image_color: " << static_cast<int>(color) << std::endl;
//     //int image_width = image[0].length();      // Width of the image (based on the first line)
//     int image_height = image.size();          // Height of the image (number of lines)

//     // Calculate the top-left position
//     int start_x = bounding_box.getXMin();
//     int start_y = bounding_box.getYMin();

//     // Print each line of the image, starting from the calculated position
//     for (int i = 0; i < image_height; ++i) {
//         Terminal::move_cursor(start_x, start_y + i);  // Move the cursor to the correct position
//         if(is_bold) {
//             Terminal::print_bold(image[i], color);//std::cout << "\033[1m" << image[i] << "\033[0m";  //print current line bold
//         }
//         else {
//             std::cout << image[i]; //print current line
//         }
//     }
//     std::cout.flush();  // Ensure the output is displayed immediately

// }
void SetPiece::draw(int client_socket) {
    // Add these debug lines at the start of draw()
    //std::cout << "is_bold: " << is_bold << std::endl;
    //std::cout << "image_color: " << static_cast<int>(color) << std::endl;
    //int image_width = image[0].length();      // Width of the image (based on the first line)
    int image_height = image.size();          // Height of the image (number of lines)

    // Calculate the top-left position
    int start_x = bounding_box.getXMin();
    int start_y = bounding_box.getYMin();

    // Print each line of the image, starting from the calculated position
    for (int i = 0; i < image_height; ++i) {
        Client_terminal::move_cursor(client_socket, start_x, start_y + i);  // Move the cursor to the correct position
        if(is_bold) {
            Client_terminal::print_bold(client_socket, image[i], color);//std::cout << "\033[1m" << image[i] << "\033[0m";  //print current line bold
        }
        else {
            //std::cout << image[i]; //print current line
            Client_terminal::print(client_socket, image[i]); //print current line
        }
    }
    //std::cout.flush();  // Ensure the output is displayed immediately

}

//overload the = operator
SetPiece& SetPiece::operator=(const SetPiece& other) {
    if (this != &other) {
        image = other.image;
        bounding_box = other.bounding_box;
        next_bounding_box = other.next_bounding_box;
        is_bold = other.is_bold;
        color = other.color;
    }
    return *this;
}