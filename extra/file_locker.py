"""
Command line utility for Windows used to lock and unlock a file
"""

import time, os

def open_file_indefinitely(file_path, open_flags):
    try:
        print("Locking file...")
        with open(file_path, open_flags):
            while True:
                time.sleep(60)
    except IOError as e:
        print("Error: {}".format(e.message))
    except KeyboardInterrupt:
        print("File unlocked using keyboard interrupt")

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description="Command line utility for Windows used to lock and unlock a file.")
    parser.add_argument("file_location", help="The file to lock")
    parser.add_argument("--create", "-c", action="store_true", help="Create the file if needed")
    args = parser.parse_args()

    file_exists = os.path.exists(args.file_location)
    
    if not file_exists and not args.create:
        print("Error: file \"{}\" does not exist, \"create (-c)\" option was not given".format(args.file_location))
        exit(-1)
    
    open_flags = 'r' if file_exists else 'x'
        
    open_file_indefinitely(args.file_location, open_flags)