# DProject

DProject is a simple docker project manager. It allow you to start, stop, restart multiple docker containers at once.

## Requirements

- docker: https://docs.docker.com/engine/installation/

## Installation

Follow the steps below to install `dproject`:

```bash
git clone 
cd dproject

sudo make install
# or
./scripts/install.sh
```

## Uninstall

To uninstall `dproject`, you can use the `uninstall` command:

```bash
sudo make uninstall
# or
./scripts/uninstall.sh
```

## Compiling manually

To compile `dproject` manually, you can use the `build` command:

```bash
make
# or
./scripts/build.sh
```

The executable will be located in the `bin` directory. You can move it to `/usr/local/bin` to use it globally:

```bash
sudo cp bin/dproject /usr/local/bin/dproject
```

## Cleanup

To clean the project, you can use the `clean` command:

```bash
make clean
```

## Usage

After installation, you can use `dproject` to manage your docker containers.

To use `dproject` with your containers, you need to rename all your containers with the following patterns:

```bash
<project_name>-<container_name>
# Example: myproject-nginx

# You can also use the following pattern:
<project_name>_<container_name>
# Example: myproject_nginx
```

So, with the following containers:

```bash
myproject-nginx
myproject-mysql
myproject-php
```

You can use the following commands:

```bash
dproject myproject start
dproject myproject stop
dproject myproject restart
```

And it will start, stop or restart all your containers.

### Options

- `-h, --help`: Display help message
- `-v, --version`: Display version

### Start

To start containers from a project, you can use the `start` command:

```bash
dproject <project_name> start
```

### Stop

To stop containers from a project, you can use the `stop` command:

```bash
dproject <project_name> stop
```

### Restart

To restart containers from a project, you can use the `restart` command:

```bash
dproject <project_name> restart
```

## Contributing

Feel free to contribute to this project by submitting a pull request.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details

## Authors

- **Hokanosekai** - *Initial work* - [Hokanosekai](https://github.com/Hokanosekai)