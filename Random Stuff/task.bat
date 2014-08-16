schtasks /RUN /TN "PlayMailer"
pause 
type C:\windows\tasks\schedlgu.txt > schtask.txt
notepad schtask.txt