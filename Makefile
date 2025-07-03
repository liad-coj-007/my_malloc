# קומפילר
CC = g++

# דגלים לקומפילציה
CFLAGS = -Wall -g

# קבצי הפלט (תוכניות ההרצה)
TARGET1 = test1
TARGET2 = test2

# קבצי מקור ל-target1
SRC1 = test1.cpp malloc_1.cpp
OBJ1 = $(SRC1:.cpp=.o)

# קבצי מקור ל-target2
SRC2 = test2.cpp malloc_2.cpp
OBJ2 = $(SRC2:.cpp=.o)

# כלל ברירת מחדל
all: $(TARGET1) $(TARGET2)

# בניית target1
$(TARGET1): $(OBJ1)
	$(CC) $(CFLAGS) $^ -o $@

# בניית target2
$(TARGET2): $(OBJ2)
	$(CC) $(CFLAGS) $^ -o $@

# כלל לקומפילציה של קבצי מקור
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# ניקוי קבצי אובייקט והפלט
clean:
	rm -f *.o $(TARGET1) $(TARGET2)
