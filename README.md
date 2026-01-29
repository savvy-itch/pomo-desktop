# Another Pomodoro App

A minimalist (i.e. primitive) pomodoro desktop app written in C with GUI written using GTK.

## Building
To build the app, run:
```
make pomodoro
```
To run the app:
```
./pomodoro
```

## Development
Before compiling the app the first time in a session, point the app to the local schemas location:
```
export GSETTINGS_SCHEMA_DIR=./data
```

After any change to schemas, run:
```
glib-compile-schemas data/
```
