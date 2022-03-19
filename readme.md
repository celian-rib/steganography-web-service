# Generated artifacts #EF656AF on runner #IUT-76678980

## binaries
binaries for different architectures may be found on the ````binaries```` subfolder

## http wrapper
Http wrapper can be found in the ````wrapper```` subfolder

## Testing
To run the application, please run ````python3 wrapper/wrapper.py```` and then encode image with the command ````curl  -F "input=@PATH-TO-FILE.bmp" -F "text=secret" http://localhost/api/encode````

to serve the web ui, run ````python3 wrapper/wrapper.py --test````  and then open http://localhost:9090

