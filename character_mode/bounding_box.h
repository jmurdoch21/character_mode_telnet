#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H

class BoundingBox {
private:
    float x_min, y_min; // Bottom-left corner
    float x_max, y_max; // Top-right corner

public:
    // Constructor
    BoundingBox(float x_min, float y_min, float x_max, float y_max) 
        : x_min(x_min), y_min(y_min), x_max(x_max), y_max(y_max) {}
    BoundingBox() : x_min(1), y_min(1), x_max(1), y_max(1) {}

    // Getters
    float getXMin() const { return x_min; }
    float getYMin() const { return y_min; }
    float getXMax() const { return x_max; }
    float getYMax() const { return y_max; }

    //dimensions
    int getWidth();
    int getHeight();

    // Calculate area of the bounding box
    float area() const;

    // Check if a point (x, y) is inside the bounding box
    bool contains(float x, float y) const;

    // Check if two bounding boxes overlap
    bool overlaps(const BoundingBox& other) const;

    // Move the bounding box by dx, dy
    void move(float dx, float dy);

    //set new origin for the bounding box
    void set(int x_min_new, int y_max_new);

    // Print bounding box coordinates
    void print() const;
};

#endif