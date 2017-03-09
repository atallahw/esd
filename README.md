# esd
EE180D Stride Detection
This is a project for EE180D, a class at UCLA on systems design.

ALWAYS:
$ git pull 
before you push

To add changes:
$ git status //make sure changes are there
$ git add .
$ git commit -am "put your comment here"
$ git push origin master -u //enter your username and password when prompted

If adding changes to a file that others are collaborating on:
BEFORE you edit the file
$ git pull
AFTER editing
$ git add . 
$ git commit -am "message"
$ git pull
If there is a conflict
$ git mergetool
If not 
$ git push origin master -u
