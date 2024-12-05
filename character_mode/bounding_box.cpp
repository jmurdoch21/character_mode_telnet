#include <iostream>
#include "bounding_box.h"

//dimensions
int BoundingBox::getWidth(){
    return x_max - x_min;
}
int BoundingBox::getHeight(){
    return y_max - y_min;
}

// Calculate area of the bounding box
float BoundingBox::area() const {
    return (x_max - x_min) * (y_max - y_min);
}

// Check if a point (x, y) is inside the bounding box
bool BoundingBox::contains(float x, float y) const {
    return x >= x_min && x <= x_max && y >= y_min && y <= y_max;
}

// Check if two bounding boxes overlap
bool BoundingBox::overlaps(const BoundingBox& other) const {
    return !(x_max < other.x_min || x_min > other.x_max || 
                y_max < other.y_min || y_min > other.y_max);
}

// Move the bounding box by dx, dy
void BoundingBox::move(float dx, float dy) {
    x_min += dx;
    x_max += dx;
    y_min += dy;
    y_max += dy;
}

// set x_min and y_min for the bounding box (top left corner)
void BoundingBox::set(int x_min_new, int y_min_new) {
    float width = getWidth();
    float height = getHeight();
    x_min = x_min_new;
    x_max = x_min + width;
    y_min = y_min_new;
    y_max = y_min + height;
}

// Print bounding box coordinates
void BoundingBox::print() const {
    std::cout << "BoundingBox: [(" << x_min << ", " << y_min << "), (" 
                << x_max << ", " << y_max << ")]" << std::endl;
}