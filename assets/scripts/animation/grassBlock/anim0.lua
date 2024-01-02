function _move01(animScript, deltaTime)
  while true do
    _Move(animScript, deltaTime, 2, 10.0, 0.0, 0.0)
    coroutine.yield()
    _Move(animScript, deltaTime, 2, 0.0, 10.0, 0.0)
    coroutine.yield()
    _Move(animScript, deltaTime, 2, 0.0, 0.0, 10.0)
    coroutine.yield()
    _Move(animScript, deltaTime, 2, 0.0, -10.0, 0.0)
    coroutine.yield()
    _Move(animScript, deltaTime, 2, -10.0, 0.0, 0.0)
    coroutine.yield()
    _Move(animScript, deltaTime, 2, 0.0, 0.0, -10.0)
    coroutine.yield()
  end
end

local animation = coroutine.create(_move01)

function nextMove(animScript, deltaTime)
  if coroutine.status(animation) ~= 'dead' then
    coroutine.resume(animation, animScript, deltaTime)
  end
end

-- no rotation
-- function nextRotation(animScript, deltaTime)
-- end