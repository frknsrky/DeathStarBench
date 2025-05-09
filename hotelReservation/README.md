# Hotel Reservation

The application implements a hotel reservation service, build with Go and gRPC, and starting from the open-source project https://github.com/harlow/go-micro-services. The initial project is extended in several ways, including adding back-end in-memory and persistent databases, adding a recommender system for obtaining hotel recommendations, and adding the functionality to place a hotel reservation. 

<!-- ## Application Structure -->

<!-- ![Social Network Architecture](socialNet_arch.png) -->

Supported actions: 
* Get profile and rates of nearby hotels available during given time periods
* Recommend hotels based on user provided metrics
* Place reservations

## Pre-requirements
- Docker
- Docker-compose
- luarocks (apt-get install luarocks)
- luasocket (luarocks install luasocket)
- libz-dev (apt-get install libz-dev)
- luasocket (sudo-apt get install luasocket)

## Running the hotel reservation application
### Before you start
- Install Docker and Docker Compose.
- Make sure exposed ports in docker-compose files are available
- Consider which platform you want to use (docker-compose/openshift/kubernetes)
    - Build the required images using the proper method
        - In case of docker-compose => docker-compose build
        - In case of Openshift => run the build script according to the readme.
        - In case of kubernetes => run the build script according to the readme.

### Running the containers
##### Docker-compose
Start docker containers by running `docker compose up -d`. All images will be pulled from Docker Hub. In order to run `docker compose` with images built from the `Dockerfile`, run `docker compose up -d --build`.

The workload itself can be configured using optional enviroment variables. The available configuration items are:

- TLS: Environment variable TLS controls the TLS enablement of gRPC and HTTP communications of the microservices in hotelReservation.
    - TLS=0 or not set(default): No TLS enabled for gRPC and HTTP communication.
    - TLS=1: All the gRPC and HTTP communications will be protected by TLS, e.g. `TLS=1 docker compose up -d`.
    - TLS=<ciphersuite>: Use specified ciphersuite for TLS, e.g. `TLS=TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256 docker- ompose up -d`. The avaialbe cipher suite can be found at the file [options.go](tls/options.go#L21).

- GC: Environment variable GC controls the garbage collection target percentage of Golang runtime. The default value is 100. See [golang doc](https://pkg.go.dev/runtime/debug#SetGCPercent) for details.

- JAEGER_SAMPLE_RATIO: Environment variable JAEGER_SAMPLE_RATIO controls the ratio of requests to be traced Jaeger. Default is 0.01(1%).

- MEMC_TIMEOUT: Environment variable MEMC_TIMEOUT controls the timeout value in seconds when communicating with memcached. Default is 2 seconds. We may need to increase this value in case of very high work loads.

- LOG_LEVEL: Environment variable LOG_LEVEL controls the log verbosity. Valid values are: ERROR, WARNING, INFO, TRACE, DEBUG. Default value is INFO.

Users may run `docker compose logs <service>` to check the corresponding configurations.

##### Openshift
Read the Readme file in Openshift directory.

##### Kubernetes
Read the Readme file in Kubernetes directory.

#### workload generation

##### Make

It is necessary to build the workload generator tool. Thus, please assert that:
1. Your repository was cloned using `--recurse-submodules` or you pulled the submodules after the clone using `git submodule update --init --recursive`
2. You have the "luajit", "luasocket", "libssl-dev" and "make" packages installed.
3. You are not running inside arm 

With the dependencies fulfilled, you can run: 

```bash
# There are two wrk2 folders: one in the root of the repository and one inside the HotelReservation folder. This is the first one.
cd ../wrk2
# Compile
make
```

And then go back to the HotelReservation folder to run the wrk2 properly for the system.
```bash
# Back to HotelReservation, in the root folder.
cd ../hotelresrvation
```

```bash
../wrk2/wrk -D exp -t <num-threads> -c <num-conns> -d <duration> -L -s ./wrk2/scripts/hotel-reservation/mixed-workload_type_1.lua http://x.x.x.x:5000 -R <reqs-per-sec>
```

### Questions and contact

You are welcome to submit a pull request if you find a bug or have extended the application in an interesting way. For any questions please contact us at: <microservices-bench-L@list.cornell.edu>


### DOCCLAB-specific modifications:

Raja (03/30/25) - Modified Docker Compose to use Jaeger 1.53 image.  HotelReservations services apart from the Jaeger container are not able to send spans with the latest version of Jaeger.  I'm unsure of versions newer than 1.53.

Raja (03/30/25) - Added lines so that Lua Hotel Reservation script looks for lua socker in the right directories on Ubuntu machines.

Raja (03/30/25) - Added more info about compiling wrk2 and running Lua scripts.  It is copied from the SocialNetwork folde.r