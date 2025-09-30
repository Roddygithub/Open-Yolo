#!/bin/bash
# Script de benchmark des performances d'Open-Yolo

set -e

BLUE='\033[0;34m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  Benchmark Performance - Open-Yolo${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Vérifier que le binaire existe
if [ ! -f "build/src/OpenYolo" ]; then
    echo -e "${YELLOW}⚠ Binaire non trouvé, compilation...${NC}"
    make build
fi

BINARY="build/src/OpenYolo"

echo -e "${YELLOW}[1/5]${NC} Informations système..."
echo "  CPU: $(lscpu | grep 'Model name' | cut -d':' -f2 | xargs)"
echo "  RAM: $(free -h | grep Mem | awk '{print $2}')"
echo "  GPU: $(lspci | grep VGA | cut -d':' -f3 | xargs)"
echo ""

echo -e "${YELLOW}[2/5]${NC} Taille du binaire..."
SIZE=$(du -h "$BINARY" | cut -f1)
SIZE_STRIPPED=$(strip --strip-all "$BINARY" -o /tmp/OpenYolo.stripped 2>/dev/null && du -h /tmp/OpenYolo.stripped | cut -f1 || echo "N/A")
echo "  Binaire : $SIZE"
echo "  Stripped : $SIZE_STRIPPED"
rm -f /tmp/OpenYolo.stripped
echo ""

echo -e "${YELLOW}[3/5]${NC} Temps de démarrage..."
START=$(date +%s%N)
timeout 2s "$BINARY" --version 2>/dev/null || true
END=$(date +%s%N)
STARTUP_TIME=$(echo "scale=3; ($END - $START) / 1000000000" | bc)
echo "  Temps : ${STARTUP_TIME}s"
echo ""

echo -e "${YELLOW}[4/5]${NC} Consommation mémoire (estimation)..."
# Lancer l'application en arrière-plan
"$BINARY" &
APP_PID=$!
sleep 2

if ps -p $APP_PID > /dev/null; then
    MEM_RSS=$(ps -o rss= -p $APP_PID | awk '{print $1/1024}')
    MEM_VSZ=$(ps -o vsz= -p $APP_PID | awk '{print $1/1024}')
    echo "  RSS : ${MEM_RSS} MB"
    echo "  VSZ : ${MEM_VSZ} MB"
    
    # Mesurer CPU pendant 5 secondes
    echo ""
    echo -e "${YELLOW}[5/5]${NC} Consommation CPU (5 secondes)..."
    CPU_SAMPLES=()
    for i in {1..5}; do
        CPU=$(ps -o %cpu= -p $APP_PID | awk '{print $1}')
        CPU_SAMPLES+=($CPU)
        echo "  Sample $i : ${CPU}%"
        sleep 1
    done
    
    # Calculer la moyenne
    CPU_AVG=$(echo "${CPU_SAMPLES[@]}" | awk '{sum=0; for(i=1;i<=NF;i++) sum+=$i; print sum/NF}')
    echo "  Moyenne : ${CPU_AVG}%"
    
    kill $APP_PID 2>/dev/null || true
else
    echo "  ⚠ Application non démarrée"
fi

echo ""
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  Résumé des Performances${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""
echo "📊 Métriques :"
echo "  • Taille binaire : $SIZE"
echo "  • Démarrage : ${STARTUP_TIME}s"
echo "  • RAM (RSS) : ${MEM_RSS} MB"
echo "  • CPU moyen : ${CPU_AVG}%"
echo ""

# Évaluation
if (( $(echo "$CPU_AVG < 5" | bc -l) )); then
    echo -e "${GREEN}✅ Performances excellentes !${NC}"
elif (( $(echo "$CPU_AVG < 10" | bc -l) )); then
    echo -e "${GREEN}✅ Performances bonnes${NC}"
else
    echo -e "${YELLOW}⚠ Performances à optimiser${NC}"
fi
echo ""
