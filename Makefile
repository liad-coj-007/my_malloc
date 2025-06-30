# קומפילר
CC = g++

# דגלים לקומפילציה
CFLAGS = -Wall -g

# הקובץ הפלט (ההפעלה)
TARGET = main

# קבצי המקור
SOURCES = malloc_1.cpp

# קבצי מקור עבור בדיקות
TEST_SOURCES = main.cpp

# קובצי אובייקט (object files)
OBJECTS = $(SOURCES:.cpp=.o) $(TEST_SOURCES:.cpp=.o)

# כלל ברירת מחדל
all: $(TARGET)

# כלל לבניית הקובץ הפלט
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

# כלל לקומפילציה של קבצי מקור
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# כלל לניקוי קבצים זמניים
clean:
	rm -f $(OBJECTS) $(TARGET)
