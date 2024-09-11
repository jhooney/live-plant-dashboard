create_network: 
	@docker network inspect plant-iot >/dev/null 2>&1 || docker network create plant-iot

up: create_network
	docker compose -f docker-compose.yaml up

down:
	docker compose -f docker-compose.yaml down

rebuild:
	docker compose -f docker-compose.yaml build --no-cache
