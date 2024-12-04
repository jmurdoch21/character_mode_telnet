#ifndef SET_PIECE_H
#define SET_PIECE_H

#include <vector>
#include <string>
#include "bounding_box.h"
#include "ansi_commands.h"  // Add at the top with other includes



class SetPiece {
    private:
        int piece_id;
        BoundingBox bounding_box;
        BoundingBox next_bounding_box;
        std::vector<std::string>& image;
        bool is_bold;
        AnsiColor color;
        float x_speed = 0; //set as speed per frame
        float y_speed = 0; //set as speed per frame

    public:
        SetPiece(BoundingBox bounding_box, int piece_id, std::vector<std::string>& image)
            : piece_id(piece_id)
            , bounding_box(bounding_box)
            , image(image) {}
        SetPiece()
            :  piece_id(0)
            , bounding_box(BoundingBox())
            , image(*(new std::vector<std::string>())) {}
        SetPiece(const SetPiece& other)
            : piece_id(other.piece_id)
            , bounding_box(other.bounding_box)
            , image(other.image) {}
        SetPiece(std::vector<std::string>& image, bool init_is_bold = false, AnsiColor init_color = AnsiColor::RESET);// : piece_id(0), image(image) {}

        int getPieceId() { return piece_id; }
        void setPieceId(int piece_id) { this->piece_id = piece_id; }

        BoundingBox getBoundingBox() { return bounding_box; }
        void setBoundingBox(BoundingBox bounding_box) { this->bounding_box = bounding_box; }
        BoundingBox getNextBoundingBox() { return next_bounding_box; }

        void set_position(int x, int y);

        int getWidth() { return bounding_box.getWidth(); }
        int getHeight() { return bounding_box.getHeight(); }

        std::vector<std::string>& getImage() { return image; }
        void setImage(std::vector<std::string>& image) { this->image = image; }
        void setBold(bool is_bold) { this->is_bold = is_bold; }
        void setColor(AnsiColor new_color) { this->color = new_color; }

        void move(float dx, float dy) {
            bounding_box.move(dx, dy);
        }

        void set_speed(float new_x_speed, float new_y_speed);
        void invert_x_speed();
        void invert_y_speed();

        void update_position();

        //void draw();
        void draw(int client_socket);

        SetPiece& operator=(const SetPiece& other);
};

#endif